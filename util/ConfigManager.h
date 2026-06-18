#include<iostream>
#include<fstream>
#include "json/value.h"

class ConfigManager{
    std::fstream fileHandle;
    std::string fileContents;
    Json::Value jsonData;
    ConfigManager();
    public:
    ConfigManager( const ConfigManager& ) = delete;
    ConfigManager& operator=(const ConfigManager& ) = delete;
    int getInt(const std::string&);
    std::string getString(const std::string&);
    bool getBool(const std::string&);

    static ConfigManager& getInstance(){
        static ConfigManager obj;
        return obj;
    }
    ~ConfigManager();
};