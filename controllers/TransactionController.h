#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class TransactionController : public drogon::HttpController<TransactionController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(TransactionController::createTransaction, "/transaction", Post);
    ADD_METHOD_TO(TransactionController::getTransaction, "/transaction/{1}", Get);
    METHOD_LIST_END

    void createTransaction(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& callback);

    void getTransaction(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& callback,
                        const std::string& id);
    static TransactionService service;
};