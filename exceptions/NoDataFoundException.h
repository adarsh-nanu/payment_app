#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class NoDataFoundException : public std::runtime_error{
public:
    NoDataFoundException(const std::string& msg) : std::runtime_error(msg){
        logger.error("Exeption: ", "NoDataFoundException", msg);
    }
};
