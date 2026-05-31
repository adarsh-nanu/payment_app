#include<iostream>
#include "HealthController.h"
#include "../util/Logger.h"

void HealthController::sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacketAlive &responsePacket){
	Json::Value resp;
    resp["Alive"]        = responsePacket.Alive;
	auto response = HttpResponse::newHttpJsonResponse(resp);
    if( responsePacket.Alive)
	    response->setStatusCode( k200OK );
    else
        response->setStatusCode( k503ServiceUnavailable );
	callback(response);
	return;
}

void HealthController::sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacketReady &responsePacket){
	Json::Value resp;
    resp["Ready"]        = responsePacket.Ready;
	auto response = HttpResponse::newHttpJsonResponse(resp);
	if( responsePacket.Ready)
	    response->setStatusCode( k200OK );
    else
        response->setStatusCode( k503ServiceUnavailable );
	callback(response);
	return;
}

void HealthController::isAlive(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
	ResponsePacketAlive responsePacket;
    responsePacket.Alive = true;
	sendResponse(callback, responsePacket );
    logger.log("Heartbeat response sent");
}

void HealthController::isReady( const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    ResponsePacketReady responsePacket;
    responsePacket.Ready =  !service.isServiceStopping() && !pool.isPoolStopping();
    sendResponse(callback, responsePacket );
    logger.log("Readystate response sent");
}