#pragma once

#include<string>
#include<iostream>
#include<fstream>
#include<mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum Mode{
NONE,
FATAL,
ERROR,
LOG,
DEBUG
};

//extern std::ostringstream oss;

class Logger{
    std::ofstream logfile;
    std::atomic<Mode> setmode;
    std::mutex mtx;
    Logger() = default;
    Logger( const Logger& ) = delete;
    void operator = ( const Logger& ) = delete;
    
    public:
    static Logger& getInstance(){
        static Logger obj;
        return obj;
    }
    void Initialize( std::string, Mode);
    void Shutdown();
    template<typename... T>
    void log(T&&... t);
    template<typename... T>
    void error(T&&... t);
    template<typename... T>
    void debug(T&&... t);
    template<typename... T>
    void fatal(T&&... t);
    bool changeLoggingMode( int);
};

template<typename... T>
void Logger::log(T&&... args)
{
    if( setmode >= LOG )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        oss << " | ";
        (oss << ... << args);
        logfile<<oss.str()<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

template<typename... T>
void Logger::debug(T&&... args)
{
    if( setmode >= DEBUG )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        oss << " | ";
        (oss << ... << args);
        logfile<<oss.str()<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

template<typename... T>
void Logger::error(T&&... args)
{
    if( setmode >= ERROR )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        oss << " | ";
        (oss << ... << args);
        logfile<<oss.str()<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

template<typename... T>
void Logger::fatal(T&&... args)
{
    if( setmode >= FATAL )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        oss << " | ";
        (oss << ... << args);
        logfile<<oss.str()<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}
extern Logger& logger;