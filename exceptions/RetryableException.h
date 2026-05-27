#pragma once
#include <stdexcept>
#include <string>

class RetryableException : public std::runtime_error{
    public:
    std::string errorCode;
public:
    RetryableException(const std::string& msg, const std::string& code) : std::runtime_error(msg), errorCode(code) {}
};
