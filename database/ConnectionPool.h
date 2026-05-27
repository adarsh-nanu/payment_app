#pragma once
#include <libpq-fe.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

class ConnectionPool{
    private:
        ConnectionPool() = default;
        int poolSize;
        std::queue<PGconn*> connections;
        std::mutex mtx;
        std::string lastErrorMessage;
        std::condition_variable cv;
        std::atomic<bool> stop{false};
        
    public:
        ~ConnectionPool();
        static ConnectionPool& getInstance();
        ConnectionPool(ConnectionPool const&) = delete;
        void operator=(ConnectionPool const&) = delete;
        int Initialize( int poolSize );
        PGconn* getConnection();
        void releaseConnection( PGconn* conn );
        std::string getErrorMessage();
        std::string className();
        void Shutdown();
};