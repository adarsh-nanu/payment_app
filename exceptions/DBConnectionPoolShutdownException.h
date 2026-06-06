#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class DBConnectionPoolShutdown : public std::runtime_error{
public:
    DBConnectionPoolShutdown(const std::string& msg) : std::runtime_error(msg){
    logger.error("Exeption: ", "DBConnectionPoolShutdown", msg);
    }
};
