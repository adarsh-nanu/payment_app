#pragma once

#include <drogon/HttpController.h>
#include "../util/Logger.h"

using namespace drogon;

class LogManagementController : public drogon::HttpController<LogManagementController>
{
    struct ResponsePacket;
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(LogManagementController::setloglevel, "/setloglevel/{1}", Post);
    METHOD_LIST_END

    void setloglevel(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& callback, int mode);

    Logger& logger = Logger::getInstance();
    void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacket &responsePacket);
};

struct LogManagementController::ResponsePacket{
    bool changed;
};