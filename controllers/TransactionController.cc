#include "TransactionController.h"
#include <libpq-fe.h>


void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, const ResponsePacket &responsePacket){
	Json::Value resp;
	resp["id"] = responsePacket.id;
	resp["success"] = responsePacket.success;
	resp["message"] = responsePacket.message;
	resp["data"] = responsePacket.data;
	if( responsePacket.error_code.length() != 0){
		resp["error_code"] = responsePacket.error_code;
	}
	auto response = HttpResponse::newHttpJsonResponse(resp);
	response->setStatusCode( responsePacket.httpStatus );
	callback(response);
	return;
}

void TransactionController::getTransaction(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& id)
{
	std::string txnStatus;

    Json::Value resp;
	ResponsePacket responsePacket;
	responsePacket.id = id;
	if( service.isStop() ){
		responsePacket.httpStatus = k503ServiceUnavailable; 
		responsePacket.message = "Service is shutting down, unable to process request";
		responsePacket.error_code = "SERVICE_UNAVAILABLE";
		sendResponse(callback, responsePacket);
		return;
	}
	if( service.getTransaction( id, txnStatus ) == -1 ){
		responsePacket.httpStatus = k500InternalServerError; 
		responsePacket.message = "DB connection failed";
		responsePacket.error_code = "SYSTEM_MALFUNCTION";
	}
	else{
		if( txnStatus.empty() ){
			responsePacket.error_code = "NOT_FOUND";
			responsePacket.httpStatus = k404NotFound;
		}
		else{
			if( txnStatus == "SUCCESS" ){
				responsePacket.success = true;
			}
			responsePacket.message= txnStatus;
			responsePacket.httpStatus = k200OK;
		}
	}
	sendResponse(callback, responsePacket );
}

void TransactionController::createTransaction(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    

	ResponsePacket responsePacket;
	if( service.isStop() ){
		responsePacket.httpStatus = k503ServiceUnavailable; 
		responsePacket.message = "Service is shutting down, unable to process request";
		responsePacket.error_code = "SERVICE_UNAVAILABLE";
		sendResponse(callback, responsePacket);
		return;
	}
	auto json = req->getJsonObject();
	if (!json)
	{
		responsePacket.httpStatus = k400BadRequest;
		responsePacket.success = false;
		responsePacket.message = "Invalid JSON request";
		responsePacket.error_code = "INVALID_FORMAT";
		sendResponse(callback, responsePacket);
		return;
	}

	TransactionJob obj;
	obj.idempotent_id = (*json)["idempotency_key"].asString();
	if( obj.idempotent_id == ""){
		responsePacket.httpStatus = k400BadRequest;
		responsePacket.success = false;
		responsePacket.message = "Idempotent key name cannot be empty";
		responsePacket.error_code = "INVALID_FORMAT";
		sendResponse(callback, responsePacket);
		return;
	}
	responsePacket.id = obj.idempotent_id;

	obj.customerName = (*json)["customer_name"].asString();
	if( obj.customerName == ""){
		responsePacket.httpStatus = k400BadRequest;
		responsePacket.success = false;
		responsePacket.message = "Customer name cannot be empty";
		responsePacket.error_code = "INVALID_FORMAT";
		sendResponse(callback, responsePacket);
		return;
	}

	obj.amount = (*json)["amount"].asDouble();
	if( obj.amount <= 0 ){
		responsePacket.httpStatus = k400BadRequest;
		responsePacket.success = false;
		responsePacket.message = "Amount must be greater than zero";
		responsePacket.error_code = "INVALID_FORMAT";
		sendResponse(callback, responsePacket);
		return;
	}

	try{
		std::cout<<"Prepare to create record in database"<<std::endl;
		service.createTransaction(obj);
	}catch(const std::exception& err ){
		responsePacket.httpStatus = k500InternalServerError; 
		responsePacket.success = false;
		responsePacket.message = "DB connection failed";
		responsePacket.error_code = "SYSTEM_MALFUNCTION";
		sendResponse(callback, responsePacket);
		return;
	}
	try{
		std::cout<<"Prepare to enqueue record for processing"<<std::endl;
		service.enqueue(obj);
	}
	catch(const std::exception &e){
		responsePacket.httpStatus = k503ServiceUnavailable; 
		responsePacket.success = false;
		responsePacket.message = "Unable to accept new messages";
		responsePacket.error_code = "BACKPRESSURE_ACTIVE";
		sendResponse(callback, responsePacket);
		return;
	}
	responsePacket.httpStatus = k200OK;
	responsePacket.success = true;
	responsePacket.message = "Request received";

	sendResponse(callback, responsePacket);
}
