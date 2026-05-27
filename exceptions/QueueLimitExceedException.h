#pragma once
#include <stdexcept>
#include <string>

class QueueLimitExceedException : public std::runtime_error{
public:
    QueueLimitExceedException(const std::string& msg) : std::runtime_error(msg){}
};
