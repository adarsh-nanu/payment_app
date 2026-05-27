#include "ConnectionPoolWrapper.h"
#include <iostream>

PGconn* ConnectionPoolWrapper::get(){
    return conn;
} 
ConnectionPoolWrapper::~ConnectionPoolWrapper(){
    if(conn){
        pool.releaseConnection(conn);
        std::cout<<"ConnectionPoolWrapper: Connection released"<<std::endl;
        //logger.log("hi");
        //logger.debug("hello");
    }
}