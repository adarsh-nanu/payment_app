#include "TransactionController.h"
#include <libpq-fe.h>

void sendResponse(std::function<void(const HttpResponsePtr&)> &callback, HttpStatusCode statuscode, bool success, const std::string &message, const std::string &errcode = "", const Json::Value &data = Json::Value()){
	Json::Value resp;
	resp["success"] = success;
	resp["message"] = message;
	if( !data.isNull()){
		resp["data"] = data;
	}
	if( errcode.length() != 0){
		resp["error_code"] = errcode;
	}
	auto response = HttpResponse::newHttpJsonResponse(resp);
	response->setStatusCode( statuscode );
	callback(response);
	return;
}

void TransactionController::getTransaction(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& id)
{
	bool success = false;
	HttpStatusCode httpStatus;
    std::cout << "GET called with id: " << id << std::endl;

    Json::Value resp;
    resp["id"] = id;

	const char* paramValues[1];
	paramValues[0] = id.c_str();

	PGconn *conn = PQconnectdb(
	"host=127.0.0.1 port=5432 dbname=payments user=postgres password=postgres123"
	);

	if (PQstatus(conn) != CONNECTION_OK)
	{
		sendResponse(callback, k500InternalServerError, false, "DB connection failed", "SYSTEM_MALFUNCTION");
		PQfinish(conn);
		return;
	}
	PGresult *res = PQexecParams(conn,
	"SELECT state  FROM transactions WHERE idempotency_key = $1",
	1,       /* number of parameters */
	NULL,    /* let the backend deduce param types */
	paramValues,
	NULL,    /* don't need lengths for text params */
	NULL,    /* default to all text params */
	0);      /* ask for text results */

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
	} else {
		// Print the first result
		//printf("User: %s, Email: %s\n", PQgetvalue(res, 0, 0), PQgetvalue(res, 0, 1));
		const char* state = PQgetvalue(res, 0, 0);
		if( strcmp(state, "SUCCESS") == 0){
				success = true;
				resp["status"] = "SUCCESS";
		}
		else{
				success = false;
				resp["status"] = state;
		}
		httpStatus = k200OK;
	}
	PQclear(res);
    auto response = HttpResponse::newHttpJsonResponse(resp);
    callback(response);
}

void TransactionController::createTransaction(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
	//std::cout<<"processing.."<<std::endl;
	Json::Value resp;
	const char *methodType = req->getMethodString();
	//std::cout<<methodType<<std::endl;
    auto json = req->getJsonObject();
	HttpStatusCode httpStatus;
	std::string error = "";
	Json::Value data; 
	bool success = false;
	PGconn *conn = PQconnectdb(
	"host=127.0.0.1 port=5432 dbname=payments user=postgres password=postgres123"
	);

	if (PQstatus(conn) != CONNECTION_OK)
	{
		sendResponse(callback, k500InternalServerError, false, "DB connection failed", "SYSTEM_MALFUNCTION");
		PQfinish(conn);
		return;
	}

	if (!json)
	{
		sendResponse(callback, k400BadRequest, false, "Invalid JSON request", "INVALID_FORMAT");
		return;
	}

	std::string customerName = (*json)["customer_name"].asString();
	if( customerName == ""){
		sendResponse(callback, k400BadRequest, false, "Customer name cannot be empty", "INVALID_JSON");
		return;
	}
	std::string amountStr = std::to_string((*json)["amount"].asDouble());
	if( (*json)["amount"].asDouble() <= 0 ){
		sendResponse(callback, k400BadRequest, false, "Amount must be greater than zero", "INVALID_JSON");
		return;
	}
	std::string idempotent_id = (*json)["idempotency_key"].asString();
	if( idempotent_id == ""){
		sendResponse(callback, k400BadRequest, false, "Idempotent key name cannot be empty", "INVALID_JSON");
		return;
	}

	
	const char *paramValues[4];
	paramValues[0] = customerName.c_str();
	paramValues[1] = amountStr.c_str();
	paramValues[2] = "PENDING";
	paramValues[3] = idempotent_id.c_str();

	PGresult *res = PQexecParams(
		conn,
		"INSERT INTO transactions "
		"(customer_name, amount, state, idempotency_key) "
		"VALUES ($1, $2, $3, $4)",
		4,          // number of params
		NULL,       // let PostgreSQL infer types
		paramValues,
		NULL,
		NULL,
		0           // text format
	);

	if (PQresultStatus(res) == PGRES_COMMAND_OK)
	{
		resp["status"] = "Transaction inserted successfully";
		httpStatus = k201Created;
		success = true;	
		data["authnum"] = "123A52";
	}
	else
	{
		const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
		if( sqlstate && !strcmp(sqlstate, "23505" ) ){
			resp["status"] = "Transaction already inserted successfully";
			httpStatus = k200OK;
			success = true;
			data["authnum"] = "723B53";
		}
		else{
			resp["status"] = "Insert failed";
			httpStatus = k500InternalServerError;
			success = false;
			error = "SYSTEM_MALFUNCTION";
			std::cout<<sqlstate<<std::endl;
		}
	}	

	PQclear(res);
	//PQclear(res);
	/*
	const char* paramValues2[1];
	paramValues2[0] = idempotent_id.c_str();
	res = PQexecParams(
		conn,
		"UPDATE transactions SET state='SUCCESS' WHERE idempotency_key=$1",
		1,          // number of params
		NULL,       // let PostgreSQL infer types
		paramValues2,
		NULL,
		NULL,
		0           // text format
	);
	PQclear(res);
	*/
	PQfinish(conn);
	sendResponse(callback, httpStatus, success, resp["status"].asString(), error, data);
}
