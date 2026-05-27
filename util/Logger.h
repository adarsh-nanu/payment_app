#pragma once

#include<string>
#include<iostream>
#include<fstream>
#include<mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include<sstream>

enum Mode{
DEBUG,
LOG
};

extern std::ostringstream oss;

class Logger{
    std::ofstream logfile;
    Mode setmode;
    std::mutex mtx;
    Logger() = default;
    Logger( const Logger& ) = delete;
    void operator = (const Logger&) = delete;
    
    public:
    static Logger& getInstance(){
        static Logger obj;
        return obj;
    }
    void log(const std::string& );
    void debug(const std::string& );
    void log( const char*);
    void debug( const char*);
    void Initialize(std::string, Mode);
    void Shutdown();
    std::string getTimeStamp();
};

//extern Logger& logger;
extern Logger& logger;