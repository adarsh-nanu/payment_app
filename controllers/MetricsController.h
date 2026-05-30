#pragma once

#include <drogon/HttpController.h>
#include "../service/TransactionService.h"
#include "../database/ConnectionPool.h"

using namespace drogon;

class MetricsController : public drogon::HttpController<MetricsController>
{
    struct ResponsePacket;
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MetricsController::getMetrics, "/metrics", Get);
    METHOD_LIST_END

    void getMetrics(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& callback);

    TransactionService& service = TransactionService::getInstance();
    ConnectionPool& pool = ConnectionPool::getInstance();
    void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacket &responsePacket);
};

struct MetricsController::ResponsePacket{
    std::size_t MessagesInQueueCount;
    std::size_t SetWorkersCount;
    std::size_t JobsInPendingCount;
    std::size_t DeadJobsCount;
    std::size_t JobsInProcessingCount;
    std::size_t JobsInFailedRetryCount;
    bool ServiceStopping;
    std::size_t AvailableConnectionsCount;
    std::size_t DBPoolSize;
    bool DBPoolStopping;
    std::size_t activeWorkerThreads;
};

//void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacket &responsePacket);