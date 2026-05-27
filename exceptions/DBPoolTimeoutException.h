#pragma once
#include <stdexcept>
#include <string>

class DBPoolTimeoutException : public std::runtime_error{
public:
    DBPoolTimeoutException(const std::string& msg) : std::runtime_error(msg){}
};
