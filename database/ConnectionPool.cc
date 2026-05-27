#include"ConnectionPool.h"
#include <iostream>
#include"../util/Logger.h"

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
        oss<<"Connection closed"<<std::endl;
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
            std::cerr << "Error occurred while initializing connection: " << e.what() << std::endl;
            return -1;
        }
    }
    return 0;
}

PGconn* ConnectionPool::getConnection(){
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] {
        return (!connections.empty() || stop);
    });
    if( stop ){
        std::cout<<"Connection pool is shutting down. No more connections can be provided."<<std::endl;
        return nullptr;
    }
    std::cout<<"Connection provided from pool. Remaining connections "<< connections.size() - 1 <<std::endl;
    PGconn* conn = connections.front();
    connections.pop();
    return conn;
}

void ConnectionPool::releaseConnection(PGconn* conn){
    std::lock_guard<std::mutex> lock(mtx);
    connections.push(conn);
    cv.notify_one();
    std::cout<<"Connection released back to pool. Available connections "<< connections.size() <<std::endl;
}

std::string ConnectionPool::getErrorMessage(){
    return lastErrorMessage;
}

std::string ConnectionPool::className(){
    return "ConnectionPool";
}