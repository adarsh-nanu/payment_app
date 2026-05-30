#include "Logger.h"

Logger& logger = Logger::getInstance();

void Logger::Initialize( std::string filename = std::string( "/tmp/a.log"), Mode _mode = LOG){
    setmode = _mode;
    logfile.open(filename, std::ios::app );
    if( !logfile.is_open() ){
        throw std::runtime_error("Unable to open log file");
    }
    logfile<<"---------Started---------"<<std::endl;
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