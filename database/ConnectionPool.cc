#include"ConnectionPool.h"
#include <iostream>
#include"../util/Logger.h"
#include <chrono>
#include "../exceptions/DBPoolTimeoutException.h"
using namespace std::chrono_literals;

ConnectionPool& ConnectionPool::getInstance(){
    static ConnectionPool instance;
    return instance;
}

void ConnectionPool::Shutdown(){
    stop = true;
    cv.notify_all(); 
}

ConnectionPool::~ConnectionPool(){
    while( !connections.empty()){
        PGconn* conn = connections.front();
        connections.pop();
        PQfinish(conn);
        oss.str("");
        oss<<"Connection closed";
        logger.log(oss.str());
    }
}

int ConnectionPool::Initialize(int poolSize){
    this->poolSize = poolSize;
    for( int i = 0; i < poolSize; i++ ){
        try{
            PGconn* conn = PQconnectdb("host=127.0.0.1 port=5432 dbname=payments user=postgres password=postgres123 connect_timeout=2");
            if( PQstatus( conn ) != CONNECTION_OK ){
                const char* sqlerrm = PQerrorMessage( conn );
                if( sqlerrm)                
                    lastErrorMessage = sqlerrm;
                return -1;
            }
            connections.push(conn);
            oss.str("");
            oss<<"Connection "<< i <<" initialized and added to pool";
            logger.debug( oss.str());
        } catch( const std::exception& e ){
            oss.str("");
            oss<<"Error occurred while initializing connection: " << e.what();
            logger.error(oss.str());
            return -1;
        }
    }
    return 0;
}

PGconn* ConnectionPool::getConnection(){
    std::unique_lock<std::mutex> lock(mtx);
    if( !cv.wait_for(lock, 2s, [this] {
        return (!connections.empty() || stop);
    }) ){
        //return nullptr;
        throw DBPoolTimeoutException("The pool cannot provide a connection at this point");
    }
    if( stop ){
        oss.str("");
        oss<<"Connection pool is shutting down. No more connections can be provided.";
        logger.log(oss.str());
        return nullptr;
    }
    oss.str("");
    oss<<"Connection provided from pool. Remaining connections "<< connections.size() - 1 ;
    logger.debug(oss.str());
    PGconn* conn = connections.front();
    connections.pop();
    return conn;
}

void ConnectionPool::releaseConnection(PGconn* conn){
    std::lock_guard<std::mutex> lock(mtx);
    connections.push(conn);
    cv.notify_one();
    oss.str("");
    oss<<"Connection released back to pool. Available connections "<< connections.size() ;
    logger.debug(oss.str());
}

std::string ConnectionPool::getErrorMessage(){
    return lastErrorMessage;
}

std::string ConnectionPool::className(){
    return "ConnectionPool";
}