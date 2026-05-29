#pragma once
#include <stdexcept>
#include <string>

class DBFetchException : public std::runtime_error{
public:
    DBFetchException(const std::string& msg) : std::runtime_error(msg){}
};
