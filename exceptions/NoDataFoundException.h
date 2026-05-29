#pragma once
#include <stdexcept>
#include <string>

class NoDataFoundException : public std::runtime_error{
public:
    NoDataFoundException(const std::string& msg) : std::runtime_error(msg){}
};
