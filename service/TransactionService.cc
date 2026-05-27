#include "TransactionService.h"
#include"../exceptions/PaymentException.h"
#include"exceptions/RetryableException.h"
#include <libpq-fe.h>
#include <iostream>
#include"../database/ConnectionPoolWrapper.h"
#include <chrono>
#include"../exceptions/PaymentException.h"
#define MAX_MSG_IN_QUEUE 5

using namespace std::chrono_literals;

TransactionService::TransactionService(int workerCount) : WorkerCount{workerCount}{
    std::cout<<"TransactionService: creating "<< WorkerCount<<" workers" <<std::endl;
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
    std::cout<<"----Start-----"<<std::endl;
    for (int i = 0; i < WorkerCount; i++) {
        workers.emplace_back(&TransactionService::workerThread, this);
        std::cout<<"Worker thread "<< i <<" started"<<std::endl;
    }
    workers.emplace_back(&TransactionService::retryWorkerThread, this);
    std::cout<<"Retry worker thread started"<<std::endl;

}

TransactionService::~TransactionService() {
    std::cout<<"Stopping worker threads"<<std::endl;
    Shutdown();
    for (auto &t : workers) {
        if (t.joinable())
            t.join();
    }
}

void TransactionService::createTransaction(const TransactionJob& obj) {
    //PGconn* conn = connectionPool.getConnection();
    ConnectionPoolWrapper conn;
    if ( conn.get() == nullptr ){
        std::cout<<"Null DB handle. "<<std::endl;
        throw std::system_error(errno, std::generic_category(), "Database connection error");
    }
    if ( PQstatus( conn.get() ) != CONNECTION_OK )
    {
        std::cout<<"Invalid DB handle. "<<std::endl;
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
    std::cout<<"Executed. check for status"<<std::endl;
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        if( sqlstate && std::strcmp(sqlstate, "23505" ) != 0 ){
            const char* sqlerrm = PQresultErrorMessage( res);
            if( sqlerrm)
                std::cout<<sqlerrm<<std::endl;
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            if( sqlstate)
                std::cout<<sqlstate<<std::endl;
                errno = 123;
            PQclear(res);
            //connectionPool.releaseConnection(conn);
            throw std::system_error(errno, std::generic_category(), "Database connection error");
        }
    }
    std::cout<<"Execution complete"<<std::endl;
    PQclear(res);
    std::cout<<"Release resources"<<std::endl;
    //connectionPool.releaseConnection(conn);
}	

void TransactionService::enqueue(const TransactionJob& obj) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout<<"Messages in the queue: "<< jobQueue.size() <<std::endl;
    if( jobQueue.size() >= MAX_MSG_IN_QUEUE ){
        std::cout<<"Cannot accept new messages."<<std::endl;
        throw PaymentException( std::string( "Cannot accept more messages" ), false, std::string( "Q_MAX_THRESHOLD_HIT" ) );
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
        std::cout<<"Worker thread waiting for job"<<std::endl;
        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [this] {
                return stop || !jobQueue.empty();
            });

            if (stop && jobQueue.empty())
            {
                std::cout<<"Worker thread stopping"<<std::endl;
                return;
            }

            job = jobQueue.front();
            jobQueue.pop();
        }

        //PGconn* conn = connectionPool.getConnection();
        ConnectionPoolWrapper conn;
        if (conn.get() == nullptr){
            throw std::system_error(errno, std::generic_category(), "Database connection error");
        }
        if (PQstatus(conn.get()) == CONNECTION_OK) {
            std::cout<<"Databse connected"<<std::endl;
            const char* params[1] = { job.idempotent_id.c_str() };
            std::cout<<"Trying to own the job "<< job.idempotent_id <<std::endl;
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
            std::cout<<PQresultStatus(res) <<std::endl;
            if( PQresultStatus(res) != PGRES_TUPLES_OK ){ 
                const char* sqlerrm = PQresultErrorMessage( res);
                if( sqlerrm)
                    std::cout<<sqlerrm<<std::endl;
                const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                if( sqlstate)
                    std::cout<<sqlstate<<std::endl;
                std::cout<<"worker db update failed "<<std::endl;
                PQclear(res);
                //connectionPool.releaseConnection(conn);
                throw RetryableException("Worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
            }else{
                std::cout<<"Check for ownership confirmation"<<std::endl;
                int rows = PQntuples(res);
                if( rows == 1){
                    std::cout<<"Job owned"<<std::endl;
                }else{
                    std::cout<<"Unable to own the job. Return"<<std::endl;
                    PQclear(res);
                    //connectionPool.releaseConnection(conn);
                    continue;
                }
            }
            PQclear(res);
        }else{
            std::cout<<"Unable to connect database"<<std::endl;
            //connectionPool.releaseConnection(conn); 
            throw RetryableException("Unable to connect database", "DB_CONNECTION_FAILED");
        }

        if (PQstatus(conn.get()) == CONNECTION_OK) {
            const char* params[1] = { job.idempotent_id.c_str() };
            std::cout<<"Updating job status "<< job.idempotent_id <<std::endl;
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
                    std::cout<<sqlerrm<<std::endl;
                const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                if( sqlstate)
                    std::cout<<sqlstate<<std::endl;
                std::cout<<"worker db update failed "<<std::endl;
                PQclear(res);
                //connectionPool.releaseConnection(conn);
                throw RetryableException("Worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
            }else{
                std::cout<<"Check for update status"<<std::endl;
                int rows = std::atoi(PQcmdTuples(res));
                if( rows != 0){
                    std::cout<<"Job updated"<<std::endl;
                }else{
                    std::cout<<"Job updated. This is not expected"<<std::endl;
                }
            }
            PQclear(res);
        }else{
            std::cout<<"Unable to connect database"<<std::endl;
            //connectionPool.releaseConnection(conn);
            throw RetryableException("Unable to connect database", "DB_CONNECTION_FAILED");  
        }

        //connectionPool.releaseConnection(conn);

        std::cout << "Processed: " << job.idempotent_id << std::endl;
        }
        catch( const RetryableException& e){
            std::cout<<"Worker thread retryable exception "<< e.what() <<std::endl;
            std::cout<<"Error code: "<< e.errorCode <<std::endl;
            setRetryFailedTransactions(job, e.what());
        }
        catch( const std::exception& e ){
            std::cout<<"Worker thread exception "<< e.what() <<std::endl;
        }
    }
}

int TransactionService::getTransaction( std::string idempotency_id, std::string &status ){
    std::cout<<"Getting transaction status for "<< idempotency_id <<std::endl;
    //PGconn* conn = connectionPool.getConnection();
    ConnectionPoolWrapper conn;
    if (conn.get() == nullptr){
        throw std::system_error(errno, std::generic_category(), "Database connection error");
    }
    if (PQstatus(conn.get()) != CONNECTION_OK)
    {
        //connectionPool.releaseConnection(conn);
        return -1;
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
        std::cout<<"Error occurred while fetching transaction status"<<std::endl;
        const char* sqlerrm = PQresultErrorMessage( res);
        if( sqlerrm)
            std::cout<<sqlerrm<<std::endl;
        const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        if( sqlstate)
            std::cout<<sqlstate<<std::endl;
        PQclear(res);
        //connectionPool.releaseConnection(conn);
        status = "";
    } else {
        if (PQntuples(res) == 0) {
            status = "";
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
            //std::cout<<"Retry worker thread checking for failed transactions"<<std::endl;
            //std::this_thread::sleep_for(std::chrono::seconds(10));
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait_for(lock, 10s, [this] { return stop.load();});
            }
            if(stop){
                std::cout<<"Retry worker thread stopping"<<std::endl;
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
                //std::cout<<"Query exec status "<<PQresultStatus(res) <<std::endl;
                if( PQresultStatus(res) != PGRES_TUPLES_OK ){ 
                    const char* sqlerrm = PQresultErrorMessage( res);
                    if( sqlerrm)
                        std::cout<<sqlerrm<<std::endl;
                    const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
                    if( sqlstate)
                        std::cout<<sqlstate<<std::endl;
                    std::cout<<"Retry worker db update failed "<<std::endl;
                    PQclear(res);
                    //connectionPool.releaseConnection(conn);
                    throw RetryableException("Retry worker db update failed", sqlstate ? sqlstate : "UNKNOWN");
                } else {
                    //std::cout<<"Retry worker db update success "<<std::endl;
                    int rows = PQntuples(res);
                    //std::cout<<"Retry worker thread found "<< rows <<" failed transactions to retry"<<std::endl;
                    for( int i = 0; i < rows; i++ ){
                        job.idempotent_id = PQgetvalue(res, i, 0);
                        job.customerName = PQgetvalue(res, i, 1);
                        job.amount = std::stod(PQgetvalue(res, i, 2));
                        int retryCount = std::atoi(PQgetvalue(res, i, 3));
                        //std::cout<<"Retrying job "<< job.idempotent_id <<" Retry count "<< retryCount <<std::endl;
                        enqueue(job);
                    }
                }
                PQclear(res);
                //connectionPool.releaseConnection(conn.get());
            }
            else{
                std::cout<<"Unable to connect database in retry worker"<<std::endl;
                //connectionPool.releaseConnection(conn);
                throw RetryableException("Unable to connect database in retry worker", "DB_CONNECTION_FAILED");
            }
            //std::this_thread::sleep_for(std::chrono::seconds(10));
            if(stop){
                std::cout<<"Retry worker thread stopping"<<std::endl;
                return; 
            }
        }catch( const RetryableException& e){
            std::cout<<"Retry worker thread retryable exception "<< e.what() <<std::endl;
            std::cout<<"Error code: "<< e.errorCode <<std::endl;
            setRetryFailedTransactions(job, e.what());
        }
        catch( const std::exception& e){
            std::cout<<"Retry worker thread exception "<< e.what() <<std::endl;
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
            if( sqlerrm)                
                std::cout<<sqlerrm<<std::endl;
            const char* sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
            if( sqlstate)                
                std::cout<<sqlstate<<std::endl;
            std::cout<<"Set retry failed transactions db update failed for "<<obj.idempotent_id<<std::endl;
            PQclear(res);
        } else {
            //std::cout<<"Set retry failed transactions db update success for "<<obj.idempotent_id<<std::endl;
        }
    } else {
        std::cout<<"Unable to connect database in set retry failed transactions"<<std::endl;
    }
    //connectionPool.releaseConnection(conn);
}