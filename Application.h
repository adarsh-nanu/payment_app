#pragma once

#include "service/TransactionService.h"
#include "database/ConnectionPool.h"
#include "util/Logger.h"


class Application{
    public:
    void Initialize();
    TransactionService& service = TransactionService::getInstance();
    ConnectionPool& connectionPool = ConnectionPool::getInstance();
    Logger& logger = Logger::getInstance();
};

void initShutDown(int);
