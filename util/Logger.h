#pragma once

#include<string>
#include<iostream>
#include<fstream>
#include<mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum Mode{
DEBUG,
LOG,
ERROR,
FATAL
};

extern std::ostringstream oss;

class Logger{
    std::ofstream logfile;
    Mode setmode;
    std::mutex mtx;
    Logger() = default;
    Logger( const Logger& ) = delete;
    void operator = ( const Logger& ) = delete;
    
    public:
    static Logger& getInstance(){
        static Logger obj;
        return obj;
    }
    void log( const std::string& );
    void debug(const std::string& );
    void log( const char* );
    void debug( const char* );
    void error( const char* );
    void error( const std::string& );
    void Initialize( std::string, Mode);
    void Shutdown();
    std::string getTimeStamp();
    template<typename... T>
    void log(T&&... t);
    template<typename... T>
    void error(T&&... t);
    template<typename... T>
    void debug(T&&... t);
};

extern Logger& logger;