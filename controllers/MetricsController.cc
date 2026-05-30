#include<iostream>
#include "MetricsController.h"
#include "../util/Logger.h"

void MetricsController::sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacket &responsePacket){
	Json::Value resp;
    resp["MessagesInQueueCount"]        = Json::UInt64( responsePacket.MessagesInQueueCount );
    resp["SetWorkersCount"]             = Json::UInt64( responsePacket.SetWorkersCount );
    resp["JobsInPendingCount"]          = Json::UInt64( responsePacket.JobsInPendingCount );
    resp["DeadJobsCount"]               = Json::UInt64( responsePacket.DeadJobsCount );
    resp["JobsInProcessingCount"]       = Json::UInt64( responsePacket.JobsInProcessingCount );
    resp["JobsInFailedRetryCount"]      = Json::UInt64( responsePacket.JobsInFailedRetryCount );
    resp["ServiceStopping"]             = responsePacket.ServiceStopping;
    resp["AvailableConnectionsCount"]   = Json::UInt64( responsePacket.AvailableConnectionsCount );
    resp["DBPoolSize"]                  = Json::UInt64( responsePacket.DBPoolSize );
    resp["DBPoolStopping"]              = responsePacket.DBPoolStopping;
    resp["ActiveThreads"]               = Json::UInt64( responsePacket.activeWorkerThreads );
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
    responsePacket.MessagesInQueueCount         = service.getMessagesInQueueCount();
    responsePacket.SetWorkersCount              = service.getSetWorkersCount();
    responsePacket.JobsInPendingCount           = service.getJobsInPendingCount();
    responsePacket.DeadJobsCount                = service.getDeadJobsCount();
    responsePacket.JobsInProcessingCount        = service.getJobsInProcessingCount();
    responsePacket.JobsInFailedRetryCount       = service.getJobsInFailedRetryCount();
    responsePacket.ServiceStopping              = service.isServiceStopping();
    responsePacket.AvailableConnectionsCount    = pool.getAvailableConnectionsCount();
    responsePacket.DBPoolSize                   = pool.getPoolSize();
    responsePacket.DBPoolStopping               = pool.isPoolStopping();
    responsePacket.activeWorkerThreads          = service.getActiveWorkerCount();
    
	sendResponse(callback, responsePacket );
}