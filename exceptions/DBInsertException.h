#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class DBInsertException : public std::runtime_error{
public:
    DBInsertException(const std::string& msg) : std::runtime_error(msg){
        logger.error("Exeption: ", "DBInsertException", msg);
    }
};
