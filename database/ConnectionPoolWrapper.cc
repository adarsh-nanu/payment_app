#include "ConnectionPoolWrapper.h"
#include <iostream>
#include "../util/Logger.h"

PGconn* ConnectionPoolWrapper::get(){
    return conn;
} 
ConnectionPoolWrapper::~ConnectionPoolWrapper(){
    if(conn){
        pool.releaseConnection(conn);
    }
}