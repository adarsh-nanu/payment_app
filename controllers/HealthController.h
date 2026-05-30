#pragma once

#include <drogon/HttpController.h>
#include "../service/TransactionService.h"
#include "../database/ConnectionPool.h"

using namespace drogon;

class HealthController : public drogon::HttpController<HealthController>
{
    struct ResponsePacketAlive;
    struct ResponsePacketReady;
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HealthController::isAlive, "/health/live", Get);
    ADD_METHOD_TO(HealthController::isReady, "/health/ready", Get);
    METHOD_LIST_END

    void isAlive(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& callback);
    void isReady(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& callback);
    TransactionService& service = TransactionService::getInstance();
    ConnectionPool& pool = ConnectionPool::getInstance();
    void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacketAlive &responsePacket);
    void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacketReady &responsePacket);
};

struct HealthController::ResponsePacketAlive{
    bool Alive;
};
struct HealthController::ResponsePacketReady{
    bool Ready;
};
//void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacket &responsePacket);