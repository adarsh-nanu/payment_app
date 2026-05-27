#include<iostream>
#include"Application.h"
#include"service/TransactionService.h"
#include "controllers/TransactionController.h"
#include<unistd.h>
#include"database/ConnectionPool.h"

void initShutDown( int signal ){
    TransactionService& service = TransactionService::getInstance();
    ConnectionPool& connectionPool = ConnectionPool::getInstance();
    oss.str("");
    oss<<"Received signal "<<signal;
    logger.log(oss.str());
    write(STDOUT_FILENO, "SIGNAL RECEIVED\n", 16);
    if( signal == SIGUSR1 ){
        oss.str("");
        oss<<"Trigerring service shutdown";
        logger.log(oss.str());
        service.Shutdown();
        oss.str("");
        oss<<"Service shutdown triggered. Waiting for ongoing transactions to complete...";
        logger.log(oss.str());
        connectionPool.Shutdown();
        oss.str("");
        oss<<"Connection pool shutdown triggered. Waiting for ongoing connections to complete...";
        logger.debug(oss.str());
        oss.str("");
        oss<<"Stopping..";
        logger.debug(oss.str());
    }
    oss.str("");
    oss<<"Stopped";
    logger.log(oss.str());
    drogon::app().quit();
    logger.Shutdown();
}

void Application::Initialize(){
    logger.Initialize("/Users/adarshnanu/drogon/build/payment_app/build/app.log", DEBUG);
    logger.log("..Initialized");
    if( connectionPool.Initialize(1) < 0 ){
        std::cerr<<"Failed to initialize connection pool: "<< connectionPool.getErrorMessage() ;
        throw std::runtime_error("Failed to initialize connection pool");
    }   
    
    signal( SIGUSR1, initShutDown );
    service.Initialize();
}