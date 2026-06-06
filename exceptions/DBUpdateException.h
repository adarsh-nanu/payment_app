#pragma once
#include <stdexcept>
#include <string>
#include "../util/Logger.h"

class DBUpdateException : public std::runtime_error{
public:
    DBUpdateException(const std::string& msg) : std::runtime_error(msg){
        logger.error("Exeption: ", "DBUpdateException", msg);
    }
};
