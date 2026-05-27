#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include<iostream>
#include "../database/ConnectionPool.h"

struct TransactionJob{
    std::string idempotent_id;
    std::string customerName;
    double amount;
};

class TransactionService {
private:
    std::queue<TransactionJob> jobQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    std::atomic<bool> stop{false};
    int WorkerCount;
    void workerThread();
    void retryWorkerThread();
    TransactionService(int workerCount = 3);
    ConnectionPool& connectionPool= ConnectionPool::getInstance();
public:
    static TransactionService& getInstance() {
        static TransactionService instance(5);
        return instance;
    }
    TransactionService(const TransactionService&) = delete;
    TransactionService& operator=(const TransactionService&) = delete;
    bool isStop() const {
        return stop;
    }
    ~TransactionService();

    void enqueue(const TransactionJob& id);
    int getTransaction( std::string idempotency_id, std::string &status );
    void createTransaction(const TransactionJob& obj);
    void setRetryFailedTransactions(const TransactionJob& obj, std::string errormesage);
    void Shutdown();
    void Initialize();
};