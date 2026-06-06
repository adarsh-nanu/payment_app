#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class DBFetchException : public std::runtime_error{
public:
    DBFetchException(const std::string& msg) : std::runtime_error(msg){
        logger.error("Exeption: ", "DBFetchException", msg);
    }
};
