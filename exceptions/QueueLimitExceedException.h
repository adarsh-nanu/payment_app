#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class QueueLimitExceedException : public std::runtime_error{
public:
    QueueLimitExceedException(const std::string& msg) : std::runtime_error(msg){
        logger.error("Exeption: ", "QueueLimitExceedException", msg);
    }
};
