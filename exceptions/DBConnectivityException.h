#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class DBConnectivityException : public std::runtime_error{
public:
    DBConnectivityException(const std::string& msg) : std::runtime_error(msg){
        logger.error("Exeption: ", "DBConnectivityException", msg);
    }
};
