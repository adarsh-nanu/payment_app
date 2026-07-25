# Payment Processing API

> A production-inspired asynchronous payment processing service built with **C++20**, **Drogon Framework**, and **PostgreSQL**.

![Architecture](Payment_app%201.0.jpg)

---

## Overview

This project demonstrates the design and implementation of a modern backend payment processing service using **C++** and the **Drogon** web framework.

The goal of the project is not simply to expose REST APIs, but to explore production-style backend engineering concepts such as:

- Asynchronous request processing
- Worker thread pools
- PostgreSQL connection pooling
- Idempotent request handling
- Retry processing
- Health monitoring
- Runtime metrics
- Structured logging
- Graceful shutdown
- Docker deployment
- Continuous Integration

Although simplified for learning purposes, the architecture closely resembles patterns used in production payment systems.

---

# Architecture

The overall architecture is shown below.

![Architecture](Payment_app%201.0.png)

### Request Flow

1. Client sends an HTTP request.
2. Drogon routes the request to the appropriate Controller.
3. The Controller validates the request and delegates business logic to the Service layer.
4. The Service stores the transaction in PostgreSQL.
5. The transaction is placed onto an in-memory queue.
6. Worker threads process queued transactions asynchronously.
7. Database connections are obtained from the Connection Pool.
8. Retry workers periodically retry failed transactions.
9. Health, Metrics and Logging components expose operational information.

---

# Features

| Feature | Description |
|----------|-------------|
| REST API | Create and retrieve payment transactions |
| PostgreSQL | Persistent transaction storage |
| Connection Pool | Efficient reuse of PostgreSQL connections |
| Worker Thread Pool | Background asynchronous processing |
| Retry Worker | Automatic retry of transient failures |
| Idempotency | Prevents duplicate transaction processing |
| Health Checks | Liveness and readiness endpoints |
| Metrics | Runtime statistics for monitoring |
| Logging | Configurable runtime log level |
| Graceful Shutdown | Clean shutdown of worker threads |
| Docker | Containerised deployment |
| GitHub Actions | Continuous Integration |

---

# Technology Stack

- C++20
- Drogon Framework
- PostgreSQL 16
- Docker
- CMake
- GitHub Actions

---

# API Endpoints

| Method | Endpoint | Description |
|---------|----------|-------------|
| POST | `/transaction` | Create a payment transaction |
| GET | `/transaction/{id}` | Retrieve a transaction |
| GET | `/health/live` | Liveness probe |
| GET | `/health/ready` | Readiness probe |
| GET | `/metrics` | Runtime metrics |
| POST | `/setloglevel/{level}` | Change application log level |

---

# Project Structure

```
payment_app/
│
├── build/
├── config/
├── controllers/
├── database/
├── health/
├── logger/
├── metrics/
├── models/
├── retry/
├── services/
├── worker/
├── Dockerfile
├── CMakeLists.txt
└── README.md
```

---

# Building the Project

Create a build directory and compile the application.

```bash
mkdir build
cd build

cmake ..
make
```

---

# Running PostgreSQL

The application uses PostgreSQL as the persistence layer.

Start PostgreSQL using Docker.

```bash
docker run -d \
  --name postgres-db \
  -e POSTGRES_USER=postgres \
  -e POSTGRES_PASSWORD=postgres123 \
  -e POSTGRES_DB=paymentdb \
  -p 5432:5432 \
  postgres:16
```

Verify that the container is running.

```bash
docker ps
```

Connect to PostgreSQL.

```bash
docker exec -it postgres-db psql -U postgres -d paymentdb
```

Once the container has been created, it can be started again using:

```bash
docker start postgres-db
```

---

# Database Schema

Create the transaction table.

```sql
CREATE TABLE transactions
(
    id SERIAL PRIMARY KEY,
    customer_name VARCHAR(100),
    amount NUMERIC(10,2),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    idempotency_key VARCHAR(100) NOT NULL UNIQUE,
    state VARCHAR(20),
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    retry INTEGER DEFAULT 0,
    last_error CHAR(100)
);
```

Verify the table.

```sql
\d transactions
```

---

# Configuration

Application settings are stored in **appsettings.json**.

```json
{
    "workerCount": 1,
    "retryWorkerIntervalSeconds": 10,
    "connectionPoolSize": 1,
    "connectionPoolTimeoutSeconds": 2,
    "maxMessagesInQueue": 100,
    "logLevel": "DEBUG",
    "dbhostname": "127.0.0.1",
    "dbport": 5432,
    "dbname": "payments",
    "dbusername": "postgres",
    "dbpassword": "postgres123"
}
```

## Configuration Options

| Setting | Description |
|----------|-------------|
| workerCount | Number of worker threads |
| retryWorkerIntervalSeconds | Retry interval |
| connectionPoolSize | Maximum number of PostgreSQL connections |
| connectionPoolTimeoutSeconds | Time to wait for a database connection |
| maxMessagesInQueue | Maximum number of queued requests |
| logLevel | Runtime logging level |
| dbhostname | PostgreSQL hostname |
| dbport | PostgreSQL port |
| dbname | Database name |
| dbusername | Database username |
| dbpassword | Database password |

---

# Docker

## Build Local Image

```bash
docker build -t payment_app .
```

## Build Docker Hub Image

Replace **adarshnanu** with your Docker Hub username.

```bash
docker build -t adarshnanu/payment_app:latest .
```

## Run Local Image

```bash
docker run \
  -e PAYMENT_APP_CONFIG=/app/config/appsettings.json \
  -e PAYMENT_APP_LOG=/app/log/payment_app.log \
  payment_app
```

## Run Docker Hub Image

```bash
docker run \
  -e PAYMENT_APP_CONFIG=/app/config/appsettings.json \
  -e PAYMENT_APP_LOG=/app/log/payment_app.log \
  adarshnanu/payment_app:latest
```

---

# Continuous Integration

The project uses **GitHub Actions** for Continuous Integration.

Every push to GitHub automatically:

- Builds the application
- Verifies that the project compiles successfully

This helps ensure that changes do not introduce build failures.

---

# Design Decisions

## Asynchronous Processing

HTTP requests are acknowledged quickly while worker threads perform background processing. This improves responsiveness and prevents clients from waiting on long-running work.

## Connection Pooling

Database connections are expensive to create. The connection pool allows worker threads to reuse existing PostgreSQL connections instead of creating new ones for every request.

## Idempotency

Each transaction includes an idempotency key.

Duplicate requests with the same key are detected, preventing accidental double processing of payments.

## Retry Processing

Retry workers periodically process transactions that previously failed due to transient errors, improving system resilience.

## Health Endpoints

Separate liveness and readiness endpoints allow orchestration platforms such as Kubernetes to determine whether the application is healthy and able to receive traffic.

---

# Future Improvements

The following enhancements are planned.

- [ ] GoogleTest unit tests
- [ ] Integration tests
- [ ] Docker Compose
- [ ] JWT authentication
- [ ] HTTPS support
- [ ] Prometheus integration
- [ ] Grafana dashboards
- [ ] OpenTelemetry tracing
- [ ] Kafka integration
- [ ] RabbitMQ integration
- [ ] Kubernetes deployment
- [ ] Database migrations

---

# Lessons Learned

This project provided practical experience in several important backend engineering topics.

- Designing asynchronous backend services
- Thread-safe programming using mutexes and condition variables
- Implementing worker thread pools
- Building a reusable PostgreSQL connection pool
- Applying RAII for resource management
- Implementing idempotent REST APIs
- Managing graceful shutdown of multithreaded applications
- Separating controller, service and infrastructure layers
- Dockerising C++ applications
- Configuring Continuous Integration using GitHub Actions

---

# Author

**Adarsh Nanu**

Backend Software Engineer

This repository was created as part of my journey to strengthen my modern C++ backend development skills and explore production-ready backend architecture using Drogon and PostgreSQL.

---

# License

This project is intended for learning, experimentation and demonstration purposes.
