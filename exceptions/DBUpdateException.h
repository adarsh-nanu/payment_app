#pragma once
#include <stdexcept>
#include <string>

class DBUpdateException : public std::runtime_error{
public:
    DBUpdateException(const std::string& msg) : std::runtime_error(msg){}
};
