#include<iostream>
#include "LogManagementController.h"
#include "../util/Logger.h"

void LogManagementController::sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacket &responsePacket){
	Json::Value resp;
    resp["changed"] = responsePacket.changed;
    auto response = HttpResponse::newHttpJsonResponse(resp);
	response->setStatusCode( k200OK );
	callback(response);
	return;
}

void LogManagementController::setloglevel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int mode
)
{
    LogManagementController::ResponsePacket responsePacket;
    responsePacket.changed = logger.changeLoggingMode(mode);
	sendResponse(callback, responsePacket );
    logger.log("Change logging ", responsePacket.changed," response sent");
}