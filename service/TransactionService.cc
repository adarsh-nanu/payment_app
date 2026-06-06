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
#include "../exceptions/DBUpdateException.h"
#include "../exceptions/DBInsertException.h"
#include "../util/ConfigManager.h"

#define MAX_MSG_IN_QUEUE 5

using namespace std::chrono_literals;

std::atomic<size_t> TransactionService::activeWorkers = 0;

ActiveWorkerTracker::ActiveWorkerTracker(){
    TransactionService::activeWorkers++;
    logger.debug("Thread active");
}

ActiveWorkerTracker::~ActiveWorkerTracker(){
    TransactionService::activeWorkers--;
    logger.debug("Thread inactive");
}

size_t TransactionService::getActiveWorkerCount(){
    return TransactionService::activeWorkers;
}

bool TransactionService::isServiceStopping(){
    return stop;
}
size_t TransactionService::getMessagesInQueueCount(){
    std::lock_guard<std::mutex> lck(mtx);
    return jobQueue.size();
}
size_t TransactionService::getSetWorkersCount(){
    return workers.size();
}
size_t TransactionService::getJobsInPendingCount(){
    try{
        ConnectionPoolWrapper conn;
        if (PQstatus(conn.get()) != CONNECTION_OK)
        {
            throw DBConnectivityException("Issue in connecting database");
        }
        const char* paramValues[1];
        PGresult *res = PQexecParams(conn.get(),
        "SELECT count(*)  FROM transactions WHERE state = 'PENDING' and retry < 3",
        0,       /* number of parameters */
        NULL,    /* let the backend deduce param types */
        NULL,
        NULL,    /* don't need lengths for text params */
        NULL,    /* default to all text params */
        0);      /* ask for text results */

        if ( PQresultStatus(res) != PGRES_TUPLES_OK ) {
            const char* sqlerrm = PQresultErrorMessage( res);
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            PQclear(res);
            throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
        } else {
            size_t count = std::stoull(PQgetvalue(res, 0, 0));
            PQclear(res);
            return count;
        }
    }
    catch(const DBFetchException& e){
        logger.error(e.what());
    }
    catch(const DBConnectivityException& e){
        logger.error(e.what());
    }
    return 0;
}
size_t TransactionService::getDeadJobsCount(){
    ConnectionPoolWrapper conn;
    try{
        if (PQstatus(conn.get()) != CONNECTION_OK)
        {
            throw DBConnectivityException("Issue in connecting database");
        }
        const char* paramValues[1];
        PGresult *res = PQexecParams(conn.get(),
        "SELECT count(*)  FROM transactions WHERE state = 'PENDING' and retry >= 3",
        0,       /* number of parameters */
        NULL,    /* let the backend deduce param types */
        NULL,
        NULL,    /* don't need lengths for text params */
        NULL,    /* default to all text params */
        0);      /* ask for text results */

        if ( PQresultStatus(res) != PGRES_TUPLES_OK ) {
            const char* sqlerrm = PQresultErrorMessage( res);
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            PQclear(res);
            throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
        } else {
            size_t count = std::stoull(PQgetvalue(res, 0, 0));
            PQclear(res);
            return count;
        }
    }
    catch(const DBFetchException& e){
        logger.error(e.what());
    }
    catch(const DBConnectivityException& e){
        logger.error(e.what());
    }
    return 0;
}

size_t TransactionService::getJobsInProcessingCount(){
    ConnectionPoolWrapper conn;
    try{
        if (PQstatus(conn.get()) != CONNECTION_OK)
        {
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
            const char* sqlerrm = PQresultErrorMessage( res);
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            PQclear(res);
            throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
        } else {
            size_t count = std::stoull(PQgetvalue(res, 0, 0));
            PQclear(res);
            return count;
        }
    }
    catch(const DBFetchException& e){
        logger.error(e.what());
    }
    catch(const DBConnectivityException& e){
        logger.error(e.what());
    }
    return 0;
}
size_t TransactionService::getJobsInFailedRetryCount(){
    ConnectionPoolWrapper conn;
    try{
        if (PQstatus(conn.get()) != CONNECTION_OK)
        {
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
            const char* sqlerrm = PQresultErrorMessage( res);
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            PQclear(res);
            throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
        } else {
            size_t count = std::stoull(PQgetvalue(res, 0, 0));
            PQclear(res);
            return count;
        }
    }
    catch(const DBFetchException& e){
        logger.error(e.what());
    }
    catch(const DBConnectivityException& e){
        logger.error(e.what());
    }
    return 0;
}

TransactionService::TransactionService(int workerCount) : WorkerCount{workerCount}{
    logger.log("TransactionService: Need to create", WorkerCount, "workers" );
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
    {
        ConfigManager configManager("/Users/adarshnanu/drogon/build/payment_app/config/appsettings.json");
        WorkerCount = configManager.getInt("workerCount");
        maxMessagesInQueue = configManager.getInt("maxMessagesInQueue");
    }
    for (int i = 0; i < WorkerCount; i++) {
        workers.emplace_back(&TransactionService::workerThread, this, i);
        logger.log("Worker thread ", i, " started" );
    }
    workers.emplace_back(&TransactionService::retryWorkerThread, this);
    logger.log("Retry worker thread started");
}

TransactionService::~TransactionService() {
    logger.log("Stopping worker threads");
    Shutdown();
    for (auto &t : workers) {
        if (t.joinable())
            t.join();
    }
}

void TransactionService::createTransaction(const TransactionJob& obj) {
    ConnectionPoolWrapper conn;
    try{
        if ( PQstatus( conn.get() ) != CONNECTION_OK )
        {
            logger.fatal("Invalid DB handle. ");
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
        logger.debug("Executed. Check for status");
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            if( sqlstate && std::strcmp(sqlstate, "23505" ) != 0 ){
                const char* sqlerrm = PQresultErrorMessage( res);
                const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                PQclear(res);
                throw DBInsertException( std::string( "Unable to insert to transactions" ) + " " + std::string( sqlerrm?sqlerrm:"") + " " + std::string( sqlstate?sqlstate:"" ) );
            }
        }
        logger.debug("Execution complete");
        PQclear(res);
        logger.debug( "Release resources" );
    }
    catch(const DBInsertException& e){
        logger.error(e.what());
    }
    catch(const DBConnectivityException& e){
        logger.error(e.what());
    }
}	

void TransactionService::enqueue(const TransactionJob& obj) {
    std::lock_guard<std::mutex> lock(mtx);
    logger.debug( "Messages in the queue: ", jobQueue.size() );
    if( jobQueue.size() >= maxMessagesInQueue ){
        throw QueueLimitExceedException( std::string( "Cannot accept more messages" ) );
    }
    jobQueue.push(obj);
    cv.notify_one();
}

void TransactionService::workerThread(int i) {
    threadName = std::string("worker-")+std::to_string(i);
    TransactionJob job;
    //pause processing to test delay
    //std::this_thread::sleep_for(std::chrono::seconds(100));
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        try{
            job = {};
            logger.debug("Worker thread waiting for job");
            {
                std::unique_lock<std::mutex> lock(mtx);

                cv.wait(lock, [this] {
                    return stop || !jobQueue.empty();
                });

                if (stop && jobQueue.empty())
                {
                    logger.debug("Worker thread stopping");
                    return;
                }

                job = jobQueue.front();
                jobQueue.pop();
            }
            ActiveWorkerTracker obj;
            ConnectionPoolWrapper conn;

            if (PQstatus(conn.get()) == CONNECTION_OK) {
                logger.debug("Databse connected");
                //std::this_thread::sleep_for(std::chrono::seconds(10));
                const char* params[1] = { job.idempotent_id.c_str() };
                logger.debug( "Trying to own the job ", job.idempotent_id );
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
                if( PQresultStatus(res) != PGRES_TUPLES_OK ){ 
                    const char* sqlerrm = PQresultErrorMessage( res);
                    const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    PQclear(res);
                    throw RetryableException("Worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
                }else{
                    logger.debug("Check for ownership confirmation");
                    int rows = PQntuples(res);
                    if( rows == 1){
                        logger.debug("Job owned");
                    }else{
                        logger.debug("Unable to own the job. Return");
                        PQclear(res);
                        continue;
                    }
                }
                PQclear(res);
            }else{
                logger.fatal("Unable to connect database");
                    logger.error(
    "PQstatus after failure=",
    PQstatus(conn.get()),
    " error=",
    PQerrorMessage(conn.get())
);
                throw RetryableException("Unable to connect database", "DB_CONNECTION_FAILED");
            }

            if (PQstatus(conn.get()) == CONNECTION_OK) {
                const char* params[1] = { job.idempotent_id.c_str() };
                logger.debug( "Updating job status ", job.idempotent_id );
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
                    const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    PQclear(res);
                    throw RetryableException("Worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
                }else{
                    logger.debug("Check for update status");
                    int rows = std::atoi(PQcmdTuples(res));
                    if( rows != 0){
                        logger.debug( "Job updated" );
                    }else{
                        logger.error( "Job updated. This is not expected" );
                    }
                }
                PQclear(res);
            }else{
                logger.fatal("Unable to connect database");
                    logger.error(
    "PQstatus after failure=",
    PQstatus(conn.get()),
    " error=",
    PQerrorMessage(conn.get())
);
                throw RetryableException("Unable to connect database", "DB_CONNECTION_FAILED");  
            }

            logger.log("Processed: ", job.idempotent_id );
        }
        catch( const RetryableException& e){
            logger.error("Worker thread retryable exception ", e.what() );
            logger.error( "Error code: ", e.errorCode );
            setRetryFailedTransactions(job, e.what());
        }
        catch( const std::exception& e ){
            logger.error("Worker thread exception ", e.what() ) ;
        }
    }
}

int TransactionService::getTransaction( std::string idempotency_id, std::string &status ){
    logger.debug("Getting transaction status for ", idempotency_id );
    ConnectionPoolWrapper conn;
    if (PQstatus(conn.get()) != CONNECTION_OK)
    {
        throw DBConnectivityException("Unable to connect database");
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
        logger.error("Error occurred while fetching transaction status" );
        const char* sqlerrm = PQresultErrorMessage( res);
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        PQclear(res);
        status = "";
        throw DBFetchException( std::string(sqlerrm?sqlerrm:"") + std::string(" ") +std::string(sqlstate?sqlstate:"") );
    } else {
        if (PQntuples(res) == 0) {
            status = "";
            PQclear(res);
            throw NoDataFoundException("Idempotency key" + idempotency_id +"not found in database");
        } else {
            status = PQgetvalue(res, 0, 0);
        }
    }

    PQclear(res);
    return 0;
}

void TransactionService::retryWorkerThread(){
    threadName = "worker-worker";
    {
        ConfigManager configManager("/Users/adarshnanu/drogon/build/payment_app/config/appsettings.json");
        retryWorkerIntervalSeconds = configManager.getInt("retryWorkerIntervalSeconds");
    }
    while( !stop ){
        TransactionJob job;
        try{
            job = {};
            // Implement retry logic for failed transactions if needed
            //std::cout<<"Retry worker thread checking for failed transactions";
            //std::this_thread::sleep_for(std::chrono::seconds(10));
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait_for(lock, std::chrono::seconds(retryWorkerIntervalSeconds), [this] { return stop.load();});
            }
            if(stop){
                logger.debug("Retry worker thread stopping");
                return;
            }
            ConnectionPoolWrapper conn;

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

                if( PQresultStatus(res) != PGRES_TUPLES_OK ){ 
                    const char* sqlerrm = PQresultErrorMessage( res);
                    const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    logger.error( "Retry worker db update failed. Status=",
                        PQresultStatus(res),
                        " Error=",
                        PQresultErrorMessage(res));
                    PQclear(res);
                    throw RetryableException("Retry worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
                } else {
                    int rows = PQntuples(res);
                    for( int i = 0; i < rows; i++ ){
                        job.idempotent_id = PQgetvalue(res, i, 0);
                        job.customerName = PQgetvalue(res, i, 1);
                        job.amount = std::stod(PQgetvalue(res, i, 2));
                        int retryCount = std::atoi(PQgetvalue(res, i, 3));
                        enqueue(job);
                    }
                }
                PQclear(res);
            }
            else{
                logger.fatal("Unable to connect database in retry worker ");
                logger.error(
    "PQstatus after failure=",
    PQstatus(conn.get()),
    " error=",
    PQerrorMessage(conn.get())
);
                throw DBConnectivityException("Unable to connect database in retry worker");
            }
            if(stop){
                logger.debug( "Retry worker thread stopping" );
                return; 
            }
        }catch( const RetryableException& e){
            logger.error( "Retry worker thread exception ", e.what() );
            try{
                //setRetryFailedTransactions();
            }catch(const std::exception& e){
                logger.error(e.what());
            }
        }
        catch( const DBConnectivityException& e ){
            logger.error( "Retry worker thread exception ", e.what() );
        }
        catch( const std::exception& e){
            logger.error( "Retry worker thread exception ", e.what() );
        }
    }
}

void TransactionService::setRetryFailedTransactions(const TransactionJob& obj , std::string errormesage){    
    ConnectionPoolWrapper conn;
    try{
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
                const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                PQclear(res);
                throw DBUpdateException("Update failed in setRetryFailedTransactions" + obj.idempotent_id);
            } else {
                logger.debug("Retry updated successfully");
            }
        } else {
                logger.error(
    "PQstatus after failure=",
    PQstatus(conn.get()),
    " error=",
    PQerrorMessage(conn.get())
);
            throw DBConnectivityException("Unable to connect database in retry worker");
        }
    }catch( const DBUpdateException& e){
        logger.error(e.what());
    }
    catch( const DBConnectivityException& e){
        logger.error(e.what());
    }
    catch(const std::exception& e){
        logger.error(e.what());
    }
}