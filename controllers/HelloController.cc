#include "HelloController.h"

void HelloController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    // write your application logic here
    Json::Value resp;
    resp["message"] = "Hello, Payment System!";
    auto response = HttpResponse::newHttpJsonResponse(resp);
    callback(response);
}

