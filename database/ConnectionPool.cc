#include"ConnectionPool.h"
#include <iostream>
#include"../util/Logger.h"
#include <chrono>
#include "../exceptions/DBPoolTimeoutException.h"
#include "../exceptions/DBConnectionPoolShutdownException.h"
#include "../exceptions/DBConnectivityException.h"
#include "../util/ConfigManager.h"

using namespace std::chrono_literals;

size_t ConnectionPool::getAvailableConnectionsCount(){
    std::lock_guard<std::mutex> lck(mtx);
    return connections.size();
}
size_t ConnectionPool::getPoolSize(){
    return poolSize;
}
bool ConnectionPool::isPoolStopping(){
    return stop;
}

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
        logger.debug("Connection closed");
    }
}

void ConnectionPool::Initialize(){
    ConfigManager& configManager = ConfigManager::getInstance();
    poolSize = configManager.getInt("connectionPoolSize");
    connectionPoolTimeoutSeconds = configManager.getInt("connectionPoolTimeoutSeconds");
    host = configManager.getString("dbhostname");
    port = configManager.getInt("dbport");
    dbname = configManager.getString("dbname");
    username = configManager.getString("dbusername");
    password = configManager.getString("dbpassword");

    for( int i = 0; i < poolSize; i++ ){
        try{
            PGconn* conn = PQconnectdb( ( std::string( "host=" ) + host + std::string( " port=" ) + std::to_string( port )+ std::string( " dbname=" ) + dbname + std::string( " user=" ) + username + std::string( " password=" ) + password + std::string( " connect_timeout=" ) + std::to_string( connectionPoolTimeoutSeconds ) ).c_str() );
            if( PQstatus( conn ) != CONNECTION_OK ){
                const char* sqlerrm = PQerrorMessage( conn );
                if( sqlerrm)                
                    lastErrorMessage = sqlerrm;
                throw DBConnectivityException( std::string("Unable to connect database") + " " + (sqlerrm?std::string( sqlerrm):"") );
            }
            connections.push(conn);
            logger.log("Connection ", i, "initialized and added to pool");
        } catch( const std::exception& e ){
            logger.fatal( "Error occurred while initializing connection: ", e.what() );
            throw DBConnectivityException("Unable to connect database");
        }
    }
}

PGconn* ConnectionPool::getConnection(){
    std::unique_lock<std::mutex> lock(mtx);
    if( !cv.wait_for(lock, std::chrono::seconds(connectionPoolTimeoutSeconds), [this] {
        return (!connections.empty() || stop);
    }) ){
        throw DBPoolTimeoutException("The pool cannot provide a connection at this point");
    }
    if( stop ){
        throw DBConnectionPoolShutdown("DB Connection pool shutting down");
    }
    logger.debug("Connection provided from pool. Remaining connections", connections.size() - 1 ) ;
    PGconn* conn = connections.front();
    connections.pop();
    return conn;
}

void ConnectionPool::releaseConnection(PGconn* conn){
    std::lock_guard<std::mutex> lock(mtx);
    connections.push(conn);
    cv.notify_one();
    logger.debug("Connection released back to pool. Available connections ", connections.size() );
}

std::string ConnectionPool::getErrorMessage(){
    return lastErrorMessage;
}

std::string ConnectionPool::className(){
    return "ConnectionPool";
}

PGconn* ConnectionPool::reConnect(){
    PGconn* conn = PQconnectdb( ( std::string( "host=" ) + host + std::string( " port=" ) + std::to_string( port )+ std::string( " dbname=" ) + dbname + std::string( " user=" ) + username + std::string( " password=" ) + password + std::string( " connect_timeout=" ) + std::to_string( connectionPoolTimeoutSeconds ) ).c_str() );
    return conn;
}