#include "Logger.h"
#include "../util/ConfigManager.h"

Logger& logger = Logger::getInstance();
thread_local std::string threadName;

Mode getMode(const std::string& Mode){
    if( Mode == "NONE" ) return NONE;
    if( Mode == "FATAL" ) return FATAL;
    if( Mode == "ERROR" ) return ERROR;
    if( Mode == "LOG" ) return LOG;
    if( Mode == "DEBUG" ) return DEBUG;
    return NONE;
}
void Logger::Initialize( std::string filename = std::string( "/tmp/a.log"), Mode _mode = LOG){
    ConfigManager configManager("/Users/adarshnanu/drogon/build/payment_app/config/appsettings.json");
    setmode = getMode( configManager.getString("logLevel") );
    //setmode = _mode;
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