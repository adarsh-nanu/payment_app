#include "ConnectionPoolWrapper.h"
#include <iostream>
#include "../util/Logger.h"
#include <libpq-fe.h>
#include <chrono>
#include<thread>
#include "../exceptions/DBConnectivityException.h"

PGconn* ConnectionPoolWrapper::get(){
    if( PQstatus( conn ) != CONNECTION_OK ){
        logger.fatal("Connection NOK, release and reset");
        PQfinish(conn);
        //std::this_thread::sleep_for(std::chrono::seconds(5));
        conn = pool.reConnect();
        if(PQstatus(conn) != CONNECTION_OK){
            throw DBConnectivityException( "Reconnect failed" );
        }
    }
    return conn;
}

ConnectionPoolWrapper::~ConnectionPoolWrapper(){
    if(conn){
        pool.releaseConnection(conn);
    }
}