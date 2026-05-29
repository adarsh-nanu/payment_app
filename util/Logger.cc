#include "Logger.h"

Logger& logger = Logger::getInstance();
std::ostringstream oss;

void Logger::Initialize( std::string filename = std::string( "/tmp/a.log"), Mode _mode = LOG){
    setmode = _mode;
    logfile.open(filename, std::ios::app );
    if( !logfile.is_open() ){
        throw std::runtime_error("Unable to open log file");
    }
    logfile<<"---------Started---------"<<std::endl;
}

template<typename... T>
void Logger::log(T&&... args)
{
    if( setmode >= LOG )
    try{
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        oss << " | ";
        (oss << ... << args);
        logfile<<oss.str();
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

template<typename... T>
void Logger::debug(T&&... args)
{
    if( setmode >= DEBUG )
    try{
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        oss << " | ";
        (oss << ... << args);
        logfile<<oss.str();
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

template<typename... T>
void Logger::error(T&&... args)
{
    if( setmode >= ERROR )
    try{
        std::ostringstream oss;
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        oss << " | ";
        (oss << ... << args);
        logfile<<oss.str();
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

void Logger::log(const std::string& data){
    if( setmode >= LOG )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        logfile<<getTimeStamp()<<" [log] "<<data<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}
void Logger::debug(const std::string& data){
    if( setmode >= DEBUG )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        logfile<<getTimeStamp()<<" [debug] "<<data<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}
void Logger::log( const char* data){
    if( setmode >= LOG )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        logfile<<getTimeStamp()<<" [log] "<<data<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}
void Logger::debug( const char* data){
    if( setmode >= DEBUG )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        logfile<<getTimeStamp()<<" [debug] "<<data<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

void Logger::Shutdown(){
    try{
        std::lock_guard<std::mutex> lock(mtx);
        logfile<<"---------Stopped---------"<<std::endl;
        logfile.close();
    }catch(const std::exception& e){
        std::cout<<"Unable to close log file"<<std::endl;
    }
}

std::string Logger::getTimeStamp(){
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%d/%m/%y %H:%M:%S");
    oss << "," << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

void Logger::error( const char* data){
    if( setmode >= LOG )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        logfile<<getTimeStamp()<<" [error] "<<data<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}

void Logger::error(const std::string& data){
    if( setmode >= DEBUG )
    try{
        std::lock_guard<std::mutex> lock(mtx);
        logfile<<getTimeStamp()<<" [eror] "<<data<<std::endl;
    }catch(const std::exception& e){
        std::cout<<"File write error"<<std::endl;
    }
}
