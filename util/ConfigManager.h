#include<iostream>
#include<fstream>
#include "json/value.h"

class ConfigManager{
    std::fstream fileHandle;
    std::string fileContents;
    Json::Value jsonData;
    public:
    ConfigManager(std::string);
    int getInt(const std::string&);
    std::string getString(const std::string&);
    bool getBool(const std::string&);
    ~ConfigManager();
};