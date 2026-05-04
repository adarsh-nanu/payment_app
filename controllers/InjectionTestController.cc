#include "InjectionTestController.h"
#include <libpq-fe.h>

void InjectionTestController::asyncHandleHttpRequest(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)> &&callback)
{
    Json::Value resp;

    auto json = req->getJsonObject();

    if (!json)
    {
        resp["status"] = "Invalid JSON request";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        callback(response);
        return;
    }

    std::string customerName = (*json)["customer_name"].asString();
    double amount = (*json)["amount"].asDouble();
    std::string status = (*json)["status"].asString();

    PGconn *conn = PQconnectdb(
        "host=127.0.0.1 port=5432 dbname=payments user=postgres password=postgres123"
    );

    if (PQstatus(conn) != CONNECTION_OK)
    {
        resp["status"] = "DB connection failed";
        auto response = HttpResponse::newHttpJsonResponse(resp);
        callback(response);
        PQfinish(conn);
        return;
    }

    std::string query =
        "INSERT INTO transactions (customer_name, amount, status) VALUES ('" +
        customerName + "', " +
        std::to_string(amount) + ", '" +
        status + "');";

    PGresult *res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        resp["status"] = "Insert failed";
    }
    else
    {
        resp["status"] = "Injection inserted successfully";
    }

    PQclear(res);
    PQfinish(conn);

    auto response = HttpResponse::newHttpJsonResponse(resp);
    callback(response);
}
