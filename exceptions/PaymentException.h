#pragma once
#include <stdexcept>
#include <string>

class PaymentException : public std::runtime_error{
    bool retryable;
    std::string errorcode;
public:
    PaymentException(const std::string& msg, bool isRetryable, const std::string& code) : std::runtime_error(msg), retryable(isRetryable), errorcode(code) {}
};
