#include<iostream>
#include"Application.h"
#include"service/TransactionService.h"
#include "controllers/TransactionController.h"
#include<unistd.h>
#include"database/ConnectionPool.h"

void initShutDown( int signal ){
    TransactionService& service = TransactionService::getInstance();
    ConnectionPool& connectionPool = ConnectionPool::getInstance();
    logger.log( "Received signal ", signal );
    write(STDOUT_FILENO, "SIGNAL RECEIVED\n", 16);
    if( signal == SIGUSR1 ){
        logger.log( "Trigerring service shutdown");
        service.Shutdown();
        logger.log( "Service shutdown triggered. Waiting for ongoing transactions to complete...");
        connectionPool.Shutdown();
        logger.log( "Connection pool shutdown triggered. Waiting for ongoing connections to complete...");
        logger.log( "Stopping.." );
    }
    logger.log( "Stopped" );
    drogon::app().quit();
    logger.Shutdown();
}

void Application::Initialize(){
    logger.Initialize("/Users/adarshnanu/drogon/build/payment_app/build/app.log", DEBUG);
    logger.log("Initializing..");
    try{
        connectionPool.Initialize();
    }catch(const std::exception& e){
        logger.debug(e.what());
        std::cerr<<"Failed to initialize connection pool: "<< connectionPool.getErrorMessage() ;
        throw std::runtime_error("Failed to initialize connection pool");
    }   
    
    signal( SIGUSR1, initShutDown );
    service.Initialize();
}