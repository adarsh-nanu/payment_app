#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

class TransactionService {
private:
    std::queue<std::string> jobQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    bool stop = false;

    void workerThread();

public:
    TransactionService(int workerCount = 3);
    ~TransactionService();

    void enqueue(const std::string& id);
};