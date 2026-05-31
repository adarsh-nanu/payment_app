#pragma once
#include <stdexcept>
#include <string>

class DBInsertException : public std::runtime_error{
public:
    DBInsertException(const std::string& msg) : std::runtime_error(msg){}
};
