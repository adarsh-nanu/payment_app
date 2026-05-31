#include "ConnectionPoolWrapper.h"
#include <iostream>
#include "../util/Logger.h"
#include <libpq-fe.h>
#include <chrono>
#include<thread>

PGconn* ConnectionPoolWrapper::get(){
    //check connection for readiness
    if( PQstatus( conn ) != CONNECTION_OK ){
        logger.fatal("Connection NOK, release and reset");
        PQfinish(conn);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        conn = pool.reConnect();
    }
    return conn;
} 
ConnectionPoolWrapper::~ConnectionPoolWrapper(){
    if(conn){
        pool.releaseConnection(conn);
    }
}