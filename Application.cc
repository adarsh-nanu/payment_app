#include<iostream>
#include"Application.h"
#include"service/TransactionService.h"
#include "controllers/TransactionController.h"
#include<unistd.h>
#include"database/ConnectionPool.h"

void initShutDown( int signal ){
    TransactionService& service = TransactionService::getInstance();
    ConnectionPool& connectionPool = ConnectionPool::getInstance();
    std::cout<<"Received signal "<<signal<<std::endl;
    write(STDOUT_FILENO, "SIGNAL RECEIVED\n", 16);
    if( signal == SIGUSR1 ){
        std::cout<<"Trigerring service shutdown"<<std::endl;
        service.Shutdown();
        std::cout<<"Service shutdown triggered. Waiting for ongoing transactions to complete..."<<std::endl;
        connectionPool.Shutdown();
        std::cout<<"Connection pool shutdown triggered. Waiting for ongoing connections to complete..."<<std::endl;
        std::cout<<"Stopping.."<<std::endl;
    }
    std::cout<<"Stopped"<<std::endl;
    drogon::app().quit();
    logger.Shutdown();
}

void Application::Initialize(){
    logger.Initialize("/Users/adarshnanu/drogon/build/payment_app/build/app.log", DEBUG);
    logger.log("..Initialized");
    if( connectionPool.Initialize(2) < 0 ){
        std::cerr<<"Failed to initialize connection pool: "<< connectionPool.getErrorMessage() <<std::endl;
        throw std::runtime_error("Failed to initialize connection pool");
    }   
    
    signal( SIGUSR1, initShutDown );
    service.Initialize();

}