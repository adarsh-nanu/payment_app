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

bool Logger::changeLoggingMode(int _mode){
    //std::lock_guard<std::mutex> lock(mtx);
    bool changed = false;
    logger.log("Current mode: ", setmode);
    switch( _mode){
        case DEBUG:
            if(setmode != DEBUG) changed = true;
            setmode = DEBUG;
            break;
        case LOG:
            if(setmode != LOG) changed = true;
            setmode = LOG;
            break;
        case ERROR:
            if(setmode != ERROR) changed = true;
            setmode = ERROR;
            break;
        case FATAL:
            if(setmode != FATAL) changed = true;
            setmode = FATAL;
            break;
        case 0:
        if(setmode != NONE) changed = true;
            setmode = NONE;
        default:
            break;
    }
    logger.log("New mode: ", setmode);
    return changed;
}