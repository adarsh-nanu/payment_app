#include "ConfigManager.h"
#include<fstream>
#include "../util/Logger.h"
#include "json/json.h"

ConfigManager::ConfigManager(){
    const char* configFile = std::getenv("PAYMENT_APP_CONFIG");
    if( configFile == nullptr)
        throw std::runtime_error("ERROR Unable to find config file");

    fileHandle = std::fstream( configFile, std::ios::in );
    if ( fileHandle.is_open() ){
        fileContents.clear();
        std::string line;
        while( std::getline( fileHandle, line) ){
            fileContents += line;
        }
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream iss(fileContents);

        if (!Json::parseFromStream(
        builder,
        iss,
        &jsonData,
        &errors))
        {
            throw std::runtime_error(
                "Invalid json contents in " +
                std::string(configFile) +
                " : " +
                errors);
        }
    }    
    else{
        throw std::runtime_error( "Unable to open file " + std::string(configFile) );
    }
}

std::string ConfigManager::getString(const std::string& parameterName){
    return jsonData[parameterName].asString();
}

int ConfigManager::getInt(const std::string& parameterName){
    return jsonData[parameterName].asInt();
}

bool ConfigManager::getBool(const std::string& parameterName){
    return jsonData[parameterName].asBool();
}

ConfigManager::~ConfigManager(){
    fileContents.clear();
    fileHandle.close();
    logger.debug("~ConfigManager");
}