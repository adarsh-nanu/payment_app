#include "TransactionService.h"
#include <libpq-fe.h>
#include <iostream>

TransactionService::TransactionService(int workerCount) {
    for (int i = 0; i < workerCount; i++) {
        workers.emplace_back(&TransactionService::workerThread, this);
    }
}

TransactionService::~TransactionService() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();

    for (auto &t : workers) {
        t.join();
    }
}

void TransactionService::enqueue(const std::string& id) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        jobQueue.push(id);
    }
    cv.notify_one();
}

void TransactionService::workerThread() {
    while (true) {
        std::string job;

        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [this] {
                return stop || !jobQueue.empty();
            });

            if (stop && jobQueue.empty())
                return;

            job = jobQueue.front();
            jobQueue.pop();
        }

        // simulate processing
        std::this_thread::sleep_for(std::chrono::seconds(5));

        PGconn* conn = PQconnectdb("host=127.0.0.1 port=5432 dbname=payments user=postgres password=postgres123");

        if (PQstatus(conn) == CONNECTION_OK) {
            const char* params[1] = { job.c_str() };

            PGresult* res = PQexecParams(
                conn,
                "UPDATE transactions SET state='SUCCESS' WHERE idempotency_key=$1",
                1,
                NULL,
                params,
                NULL,
                NULL,
                0
            );

            PQclear(res);
        }

        PQfinish(conn);

        std::cout << "Processed: " << job << std::endl;
    }
}