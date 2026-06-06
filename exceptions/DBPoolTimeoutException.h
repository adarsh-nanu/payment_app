#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class DBPoolTimeoutException : public std::runtime_error{
public:
    DBPoolTimeoutException(const std::string& msg) : std::runtime_error(msg){
        logger.error("Exeption: ", "DBPoolTimeoutException", msg);
    }
};
