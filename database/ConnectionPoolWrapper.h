#pragma once

#include "ConnectionPool.h"
#include <iostream>
//#include"../util/Logger.h"

class ConnectionPoolWrapper{
    PGconn* conn;
    ConnectionPool& pool;
    ConnectionPoolWrapper(const ConnectionPoolWrapper&) = delete;
    void operator = (const ConnectionPoolWrapper&) = delete;
    public:
    ConnectionPoolWrapper(): pool{ConnectionPool::getInstance()}{
        conn = pool.getConnection();
        std::cout<<"ConnectionPoolWrapper: "<<std::endl;
    }
    PGconn* get(); 
    ~ConnectionPoolWrapper();
};