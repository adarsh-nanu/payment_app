#pragma once
#include <stdexcept>
#include <string>

class DBConnectionPoolShutdown : public std::runtime_error{
public:
    DBConnectionPoolShutdown(const std::string& msg) : std::runtime_error(msg){}
};
