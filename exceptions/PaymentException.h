#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class PaymentException : public std::runtime_error{
    bool retryable;
    std::string errorcode;
public:
    PaymentException(const std::string& msg, bool isRetryable, const std::string& code) : std::runtime_error(msg), retryable(isRetryable), errorcode(code) {
        logger.error("Exeption: ", "PaymentException", msg);
    }
};
