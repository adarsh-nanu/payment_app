#pragma once
#include <stdexcept>
#include <string>

class DBConnectivityException : public std::runtime_error{
public:
    DBConnectivityException(const std::string& msg) : std::runtime_error(msg){}
};
