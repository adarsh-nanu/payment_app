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

extern thread_local std::string threadName;

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
    template<typename... T>
    void write(T&&... t);
};

template<typename... T>
void Logger::write(T&&... args)
{
    try{
        std::lock_guard<std::mutex> lock(mtx);
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        oss << ',';
        oss << std::setfill('0');
        oss << std::setw(3);
        oss << ms.count();
        oss << " [";
        oss << threadName;
        oss << "] ";
        ( ( oss << args << ' '), ...);
        logfile<<oss.str()<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

template<typename... T>
void Logger::log(T&&... args){
    if( setmode >= LOG )
        write( std::forward<T>(args)... );
}

template<typename... T>
void Logger::debug(T&&... args)
{
    if( setmode >= DEBUG )
        write( std::forward<T>(args)... );
}

template<typename... T>
void Logger::error(T&&... args)
{
    if( setmode >= ERROR )
        write( std::forward<T>(args)... );
}

template<typename... T>
void Logger::fatal(T&&... args)
{
    if( setmode >= FATAL )
        write( std::forward<T>(args)... );
}

extern Logger& logger;
