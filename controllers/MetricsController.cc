#include<iostream>
#include "MetricsController.h"
#include "../util/Logger.h"

void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacket &responsePacket){
	Json::Value resp;
    resp["MessagesInQueueCount"] = std::to_string(responsePacket.MessagesInQueueCount);
    resp["ActiveWorkersCount"] = std::to_string(responsePacket.ActiveWorkersCount);
    resp["JobsInPendingCount"] = std::to_string(responsePacket.JobsInPendingCount);
    resp["JobsInProcessingCount"] = std::to_string(responsePacket.JobsInProcessingCount);
    resp["JobsInFailedRetryCount"] = std::to_string(responsePacket.JobsInFailedRetryCount);
    resp["ServiceStopping"] = responsePacket.ServiceStopping;
    resp["AvailableConnectionsCount"] = std::to_string(responsePacket.AvailableConnectionsCount);
    resp["DBPoolSize"] = std::to_string(responsePacket.DBPoolSize);
    resp["DBPoolStopping"] = responsePacket.DBPoolStopping;
	auto response = HttpResponse::newHttpJsonResponse(resp);
	response->setStatusCode( k200OK );
	callback(response);
	return;
}

void MetricsController::getMetrics(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{

	ResponsePacket responsePacket;
    responsePacket.MessagesInQueueCount = service.getMessagesInQueueCount();
    responsePacket.ActiveWorkersCount = service.getWorkersCount();
    responsePacket.JobsInPendingCount = service.getJobsInPendingCount();
    responsePacket.JobsInProcessingCount = service.getJobsInProcessingCount();
    responsePacket.JobsInFailedRetryCount = service.getJobsInFailedRetryCount();
    responsePacket.ServiceStopping = service.isServiceStopping();
    responsePacket.AvailableConnectionsCount = pool.getAvailableConnectionsCount();
    responsePacket.DBPoolSize = pool.getPoolSize();
    responsePacket.DBPoolStopping = pool.isPoolStopping();
    
	sendResponse(callback, responsePacket );
}