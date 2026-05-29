#include "TransactionService.h"
#include "../exceptions/PaymentException.h"
#include "exceptions/RetryableException.h"
#include <libpq-fe.h>
#include <iostream>
#include "../database/ConnectionPoolWrapper.h"
#include <chrono>
#include "../util/Logger.h"
#include "../exceptions/PaymentException.h"
#include "../exceptions/QueueLimitExceedException.h"
#include "../exceptions/DBConnectivityException.h"
#include "../exceptions/DBFetchException.h"
#include "../exceptions/NoDataFoundException.h"

#define MAX_MSG_IN_QUEUE 5

using namespace std::chrono_literals;

bool TransactionService::isServiceStopping(){
    return stop;
}
size_t TransactionService::getMessagesInQueueCount(){
    return jobQueue.size();
}
size_t TransactionService::getWorkersCount(){
    return workers.size();
}
size_t TransactionService::getJobsInPendingCount(){
    ConnectionPoolWrapper conn;
    if (PQstatus(conn.get()) != CONNECTION_OK)
    {
        //connectionPool.releaseConnection(conn);
        throw DBConnectivityException("Issue in connecting database");
    }
    const char* paramValues[1];
    PGresult *res = PQexecParams(conn.get(),
    "SELECT count(*)  FROM transactions WHERE state = 'PENDING' and retry <= 3",
    0,       /* number of parameters */
    NULL,    /* let the backend deduce param types */
    NULL,
    NULL,    /* don't need lengths for text params */
    NULL,    /* default to all text params */
    0);      /* ask for text results */

    if ( PQresultStatus(res) != PGRES_TUPLES_OK ) {
        logger.log("Error occurred while fetching transaction status");
        const char* sqlerrm = PQresultErrorMessage( res);
        if( sqlerrm){
            oss.str("");
            oss<<sqlerrm;
            logger.error(oss.str());
        }
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        if( sqlstate){
            oss.str("");
            oss<<sqlstate;
            logger.error(oss.str());
        }
        PQclear(res);
        logger.error("Unable to fetch from database", sqlerrm?sqlerrm:"", sqlstate?sqlstate:sqlstate);
        throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
    } else {
        size_t count = std::stoull(PQgetvalue(res, 0, 0));
        PQclear(res);
        return count;
    }
    return 0;
}

size_t TransactionService::getJobsInProcessingCount(){
    ConnectionPoolWrapper conn;
    if (PQstatus(conn.get()) != CONNECTION_OK)
    {
        //connectionPool.releaseConnection(conn);
        throw DBConnectivityException("Issue in connecting database");
    }
    const char* paramValues[1];
    PGresult *res = PQexecParams(conn.get(),
    "SELECT count(*)  FROM transactions WHERE state = 'PROCESSING' and retry <= 3",
    0,       /* number of parameters */
    NULL,    /* let the backend deduce param types */
    NULL,
    NULL,    /* don't need lengths for text params */
    NULL,    /* default to all text params */
    0);      /* ask for text results */

    if ( PQresultStatus(res) != PGRES_TUPLES_OK ) {
        logger.log("Error occurred while fetching transaction status");
        const char* sqlerrm = PQresultErrorMessage( res);
        if( sqlerrm){
            oss.str("");
            oss<<sqlerrm;
            logger.error(oss.str());
        }
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        if( sqlstate){
            oss.str("");
            oss<<sqlstate;
            logger.error(oss.str());
        }
        PQclear(res);
        logger.error("Unable to fetch from database", sqlerrm?sqlerrm:"", sqlstate?sqlstate:sqlstate);
        throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
    } else {
        size_t count = std::stoull(PQgetvalue(res, 0, 0));
        PQclear(res);
        return count;
    }
    return 0;
}
size_t TransactionService::getJobsInFailedRetryCount(){
    ConnectionPoolWrapper conn;
    if (PQstatus(conn.get()) != CONNECTION_OK)
    {
        //connectionPool.releaseConnection(conn);
        throw DBConnectivityException("Issue in connecting database");
    }
    const char* paramValues[1];
    PGresult *res = PQexecParams(conn.get(),
    "SELECT count(*)  FROM transactions WHERE state = 'FAILED_RETRY' and retry <= 3",
    0,       /* number of parameters */
    NULL,    /* let the backend deduce param types */
    NULL,
    NULL,    /* don't need lengths for text params */
    NULL,    /* default to all text params */
    0);      /* ask for text results */

    if ( PQresultStatus(res) != PGRES_TUPLES_OK ) {
        logger.log("Error occurred while fetching transaction status");
        const char* sqlerrm = PQresultErrorMessage( res);
        if( sqlerrm){
            oss.str("");
            oss<<sqlerrm;
            logger.error(oss.str());
        }
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        if( sqlstate){
            oss.str("");
            oss<<sqlstate;
            logger.error(oss.str());
        }
        PQclear(res);
        logger.error("Unable to fetch from database", sqlerrm?sqlerrm:"", sqlstate?sqlstate:sqlstate);
        throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
    } else {
        size_t count = std::stoull(PQgetvalue(res, 0, 0));
        PQclear(res);
        return count;
    }
    return 0;
}

TransactionService::TransactionService(int workerCount) : WorkerCount{workerCount}{
    oss.str("");
    oss<<"TransactionService: Need to create "<< WorkerCount<<" workers" ;
    logger.debug(oss.str());
};

void TransactionService::Shutdown(){
    {
        std::lock_guard<std::mutex> lock(mtx);
        TransactionService::stop = true;
    }
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    cv.notify_all();
}

void TransactionService::Initialize() {
    std::cout<<"----Start-----";
    for (int i = 0; i < WorkerCount; i++) {
        workers.emplace_back(&TransactionService::workerThread, this);
        oss.str("");
        oss<<"Worker thread "<< i <<" started";
        logger.log(oss.str());
    }
    workers.emplace_back(&TransactionService::retryWorkerThread, this);
    oss.str("");
    oss<<"Retry worker thread started";
    logger.log(oss.str());
}

TransactionService::~TransactionService() {
    oss.str("");
    oss<<"Stopping worker threads";
    logger.log(oss.str());
    Shutdown();
    for (auto &t : workers) {
        if (t.joinable())
            t.join();
    }
}

void TransactionService::createTransaction(const TransactionJob& obj) {
    //PGconn* conn = connectionPool.getConnection();
    ConnectionPoolWrapper conn;
    /*
    if ( conn.get() == nullptr ){
        oss.str("");
        oss<<"Null DB handle. ";
        logger.error(oss.str());
        throw std::system_error(errno, std::generic_category(), "Database connection error");
    }*/
    if ( PQstatus( conn.get() ) != CONNECTION_OK )
    {
        oss.str("");
        oss<<"Invalid DB handle. ";
        logger.error(oss.str());
        //connectionPool.releaseConnection(conn);
        throw std::system_error(errno, std::generic_category(), "Database connection error");
    }
    const char* paramValues[4];
    paramValues[0] = obj.customerName.c_str();
    std::string amountStr = std::to_string(obj.amount);
    paramValues[1] = amountStr.c_str();
    paramValues[2] = "PENDING";
    paramValues[3] = obj.idempotent_id.c_str();

    PGresult *res = PQexecParams(
    conn.get(),
    "INSERT INTO transactions "
    "(customer_name, amount, state, idempotency_key) "
    "VALUES ($1, $2, $3, $4)",
    4,          // number of params
    NULL,       // let PostgreSQL infer types
    paramValues,
    NULL,
    NULL,
    0           // text format
    );
    oss.str("");
    oss<<"Executed. Check for status";
    logger.debug(oss.str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        if( sqlstate && std::strcmp(sqlstate, "23505" ) != 0 ){
            const char* sqlerrm = PQresultErrorMessage( res);
            if( sqlerrm)
                std::cout<<sqlerrm;
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            if( sqlstate)
                std::cout<<sqlstate;
                errno = 123;
            PQclear(res);
            //connectionPool.releaseConnection(conn);
            throw std::system_error(errno, std::generic_category(), "Database connection error");
        }
    }
    oss.str("");
    oss<<"Execution complete";
    logger.debug(oss.str());
    PQclear(res);
    oss.str("");
    oss<<"Release resources";
    logger.debug(oss.str());
    //connectionPool.releaseConnection(conn);
}	

void TransactionService::enqueue(const TransactionJob& obj) {
    std::lock_guard<std::mutex> lock(mtx);
    oss.str("");
    oss<<"Messages in the queue: "<< jobQueue.size() ;
    logger.debug(oss.str());
    if( jobQueue.size() >= MAX_MSG_IN_QUEUE ){
        oss.str("");
        oss<<"Cannot accept new messages.";
        logger.log(oss.str());
        throw QueueLimitExceedException( std::string( "Cannot accept more messages" ) );
    }
    jobQueue.push(obj);
    cv.notify_one();
}

void TransactionService::workerThread() {
    TransactionJob job;
    //pause processing to test delay
    //std::this_thread::sleep_for(std::chrono::seconds(100));
    while (true) {
        try{
            job = {};
            oss.str("");
            oss<<"Worker thread waiting for job";
            logger.debug(oss.str());
            {
                std::unique_lock<std::mutex> lock(mtx);

                cv.wait(lock, [this] {
                    return stop || !jobQueue.empty();
                });

                if (stop && jobQueue.empty())
                {
                    oss.str("");
                    oss<<"Worker thread stopping";
                    logger.log(oss.str());
                    return;
                }

                job = jobQueue.front();
                jobQueue.pop();
            }

            //PGconn* conn = connectionPool.getConnection();
            ConnectionPoolWrapper conn;
            /*if (conn.get() == nullptr){
                throw std::system_error(errno, std::generic_category(), "Database connection error");
            }*/
            if (PQstatus(conn.get()) == CONNECTION_OK) {
                oss.str("");
                oss<<"Databse connected";
                logger.debug(oss.str());
    std::this_thread::sleep_for(std::chrono::seconds(10));
                const char* params[1] = { job.idempotent_id.c_str() };
                oss.str("");
                oss<<"Trying to own the job "<< job.idempotent_id ;
                logger.debug(oss.str());
                PGresult* res = PQexecParams(
                    conn.get(),
                    "UPDATE transactions SET state='PROCESSING', updated_at = now() WHERE idempotency_key=$1 and state = 'PENDING' returning idempotency_key",
                    1,
                    NULL,
                    params,
                    NULL,
                    NULL,
                    0
                );
                std::cout<<PQresultStatus(res) ;
                if( PQresultStatus(res) != PGRES_TUPLES_OK ){ 
                    const char* sqlerrm = PQresultErrorMessage( res);
                    if( sqlerrm)
                        std::cout<<sqlerrm;
                    const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    if( sqlstate)
                        std::cout<<sqlstate;
                    oss.str("");
                    oss<<"worker db update failed ";
                    logger.error(oss.str());
                    PQclear(res);
                    //connectionPool.releaseConnection(conn);
                    throw RetryableException("Worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
                }else{
                    oss.str("");
                    oss<<"Check for ownership confirmation";
                    logger.debug(oss.str());
                    int rows = PQntuples(res);
                    if( rows == 1){
                        oss.str("");
                        oss<<"Job owned";
                        logger.debug(oss.str());
                    }else{
                        oss.str("");
                        oss<<"Unable to own the job. Return";
                        logger.log(oss.str());
                        PQclear(res);
                        //connectionPool.releaseConnection(conn);
                        continue;
                    }
                }
                PQclear(res);
            }else{
                oss.str("");
                oss<<"Unable to connect database";
                logger.error(oss.str());
                //connectionPool.releaseConnection(conn); 
                throw RetryableException("Unable to connect database", "DB_CONNECTION_FAILED");
            }

            if (PQstatus(conn.get()) == CONNECTION_OK) {
                const char* params[1] = { job.idempotent_id.c_str() };
                oss.str("");
                oss<<"Updating job status "<< job.idempotent_id ;
                logger.debug(oss.str());
                PGresult* res = PQexecParams(
                    conn.get(),
                    "UPDATE transactions SET state='SUCCESS', updated_at = now() WHERE idempotency_key=$1 and state = 'PROCESSING'",
                    1,
                    NULL,
                    params,
                    NULL,
                    NULL,
                    0
                );
                if( PQresultStatus(res) != PGRES_COMMAND_OK ){
                    const char* sqlerrm = PQresultErrorMessage( res);
                    if( sqlerrm)
                        std::cout<<sqlerrm;
                    const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    if( sqlstate)
                        std::cout<<sqlstate;
                    oss.str("");
                    oss<<"worker db update failed ";
                    logger.error(oss.str());
                    PQclear(res);
                    //connectionPool.releaseConnection(conn);
                    throw RetryableException("Worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
                }else{
                    oss.str("");
                    oss<<"Check for update status";
                    logger.debug(oss.str());
                    int rows = std::atoi(PQcmdTuples(res));
                    if( rows != 0){
                        oss.str("");
                        oss<<"Job updated";
                        logger.debug(oss.str());
                    }else{
                        oss.str("");
                        oss<<"Job updated. This is not expected";
                        logger.log(oss.str());
                    }
                }
                PQclear(res);
            }else{
                oss.str("");
                oss<<"Unable to connect database";
                logger.error(oss.str());
                //connectionPool.releaseConnection(conn);
                throw RetryableException("Unable to connect database", "DB_CONNECTION_FAILED");  
            }

            //connectionPool.releaseConnection(conn);

            oss.str("");
            oss<< "Processed: " << job.idempotent_id;
            logger.debug(oss.str());
        }
        catch( const RetryableException& e){
            oss.str("");
            oss<<"Worker thread retryable exception "<< e.what() ;
            logger.error(oss.str());
            oss.str("");
            oss<<"Error code: "<< e.errorCode ;
            logger.error(oss.str());
            setRetryFailedTransactions(job, e.what());
        }
        catch( const std::exception& e ){
            oss.str("");
            oss<<"Worker thread exception "<< e.what() ;
            logger.error(oss.str());
        }
    }
}

int TransactionService::getTransaction( std::string idempotency_id, std::string &status ){
    oss.str("");
            oss<<"Getting transaction status for "<< idempotency_id ;
            logger.debug(oss.str());
    //PGconn* conn = connectionPool.getConnection();
    ConnectionPoolWrapper conn;
    /*if (conn.get() == nullptr){
        throw std::system_error(errno, std::generic_category(), "Database connection error");
    }*/
    if (PQstatus(conn.get()) != CONNECTION_OK)
    {
        //connectionPool.releaseConnection(conn);
        throw DBConnectivityException("Issue in connecting database");
    }
    const char* paramValues[1];
    paramValues[0] = idempotency_id.c_str();
    PGresult *res = PQexecParams(conn.get(),
    "SELECT state  FROM transactions WHERE idempotency_key = $1",
    1,       /* number of parameters */
    NULL,    /* let the backend deduce param types */
    paramValues,
    NULL,    /* don't need lengths for text params */
    NULL,    /* default to all text params */
    0);      /* ask for text results */

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        oss.str("");
        oss<<"Error occurred while fetching transaction status";
        logger.error(oss.str());
        const char* sqlerrm = PQresultErrorMessage( res);
        if( sqlerrm){
            oss.str("");
            oss<<sqlerrm;
            logger.error(oss.str());
        }
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        if( sqlstate){
            oss.str("");
            oss<<sqlstate;
            logger.error(oss.str());
        }
        PQclear(res);
        //connectionPool.releaseConnection(conn);
        status = "";
        throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
    } else {
        if (PQntuples(res) == 0) {
            status = "";
            PQclear(res);
            throw NoDataFoundException("Idempotency key not found in database");
        } else {
            status = PQgetvalue(res, 0, 0);
        }
    }

    PQclear(res);
    //connectionPool.releaseConnection(conn);
    return 0;
}

void TransactionService::retryWorkerThread(){
    while( !stop ){
        TransactionJob job;
        try{
            job = {};
            // Implement retry logic for failed transactions if needed
            //std::cout<<"Retry worker thread checking for failed transactions";
            //std::this_thread::sleep_for(std::chrono::seconds(10));
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait_for(lock, 10s, [this] { return stop.load();});
            }
            if(stop){
                oss.str("");
                oss<<"Retry worker thread stopping";
                logger.log(oss.str());
                return;
            }
            //PGconn* conn = connectionPool.getConnection();
            ConnectionPoolWrapper conn;
            if (conn.get() == nullptr){
                throw std::system_error(errno, std::generic_category(), "Database connection error");
            }
            if( PQstatus(conn.get()) == CONNECTION_OK ){
                PGresult* res = PQexecParams(
                    conn.get(),
                    "UPDATE transactions SET state='PENDING', updated_at = now(), retry = coalesce(retry, 0) + 1 WHERE \
                    state in( 'FAILED_RETRYABLE', 'PENDING' ) and \
                    updated_at <  ( now() - interval '15 seconds' ) and \
                    coalesce(retry, 0) < 3 \
                    returning idempotency_key, customer_name, amount, retry",
                    0,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    0
                );
                //std::cout<<"Query exec status "<<PQresultStatus(res) ;
                if( PQresultStatus(res) != PGRES_TUPLES_OK ){ 
                    const char* sqlerrm = PQresultErrorMessage( res);
                    if( sqlerrm)
                        std::cout<<sqlerrm;
                    const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    if( sqlstate)
                        std::cout<<sqlstate;
                    oss.str("");
                    oss<<"Retry worker db update failed ";
                    logger.error(oss.str());
                    PQclear(res);
                    //connectionPool.releaseConnection(conn);
                    throw RetryableException("Retry worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
                } else {
                    //std::cout<<"Retry worker db update success ";
                    int rows = PQntuples(res);
                    //std::cout<<"Retry worker thread found "<< rows <<" failed transactions to retry";
                    for( int i = 0; i < rows; i++ ){
                        job.idempotent_id = PQgetvalue(res, i, 0);
                        job.customerName = PQgetvalue(res, i, 1);
                        job.amount = std::stod(PQgetvalue(res, i, 2));
                        int retryCount = std::atoi(PQgetvalue(res, i, 3));
                        //std::cout<<"Retrying job "<< job.idempotent_id <<" Retry count "<< retryCount ;
                        enqueue(job);
                    }
                }
                PQclear(res);
                //connectionPool.releaseConnection(conn.get());
            }
            else{
                oss.str("");
                oss<<"Unable to connect database in retry worker";
                logger.error(oss.str());
                //connectionPool.releaseConnection(conn);
                throw RetryableException("Unable to connect database in retry worker", "DB_CONNECTION_FAILED");
            }
            //std::this_thread::sleep_for(std::chrono::seconds(10));
            if(stop){
                oss.str("");
                oss<<"Retry worker thread stopping";
                logger.error(oss.str());
                return; 
            }
        }catch( const RetryableException& e){
            oss.str("");
            oss<<"Retry worker thread retryable exception "<< e.what() ;
            logger.error(oss.str());
            oss.str("");
            oss<<"Error code: "<< e.errorCode ;
            logger.error(oss.str());
            setRetryFailedTransactions(job, e.what());
        }
        catch( const std::exception& e){
            oss.str("");
            oss<<"Retry worker thread exception "<< e.what() ;
            logger.error(oss.str());
        }
    }
}

void TransactionService::setRetryFailedTransactions(const TransactionJob& obj , std::string errormesage){    
    //PGconn* conn = connectionPool.getConnection();
    ConnectionPoolWrapper conn;
    if (conn.get() == nullptr){
        throw std::system_error(errno, std::generic_category(), "Database connection error");
    }
    if( PQstatus(conn.get()) == CONNECTION_OK ){
        const char* paramValues[2];
        paramValues[0] = obj.idempotent_id.c_str();
        paramValues[1] = errormesage.c_str();
        PGresult* res = PQexecParams(
            conn.get(),
            "UPDATE transactions SET state='FAILED_RETRYABLE', updated_at = now(), last_error = $2 WHERE \
            state = 'PROCESSING' and idempotency_key = $1",
            2,
            NULL,
            paramValues,
            NULL,
            NULL,
            0
        );
        if( PQresultStatus(res) != PGRES_COMMAND_OK ){
            const char* sqlerrm = PQresultErrorMessage( res);
            if( sqlerrm){                
                oss.str("");
                oss<<sqlerrm;
                logger.error(oss.str());
            }
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            if( sqlstate){                
                oss.str("");
                oss<<sqlstate;
                logger.error(oss.str());
            }
            oss.str("");
            oss<<"Set retry failed transactions db update failed for "<<obj.idempotent_id;
            logger.error(oss.str());
            PQclear(res);
        } else {
            //std::cout<<"Set retry failed transactions db update success for "<<obj.idempotent_id;
        }
    } else {
        oss.str("");
        oss<<"Unable to connect database in set retry failed transactions";
        logger.error(oss.str());
    }
    //connectionPool.releaseConnection(conn);
}