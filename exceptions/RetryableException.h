#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class RetryableException : public std::runtime_error{
    public:
    std::string errorCode;
public:
    RetryableException(const std::string& msg, const std::string& code) : std::runtime_error(msg), errorCode(code) {
        logger.error("Exeption: ", "RetryableException", msg);
    }
};
