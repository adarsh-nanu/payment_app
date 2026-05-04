#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class InjectionTestController
    : public drogon::HttpSimpleController<InjectionTestController>
{
  public:
    void asyncHandleHttpRequest(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)> &&callback
    ) override;

    PATH_LIST_BEGIN
    PATH_ADD("/injection", Post);
    PATH_LIST_END
};
