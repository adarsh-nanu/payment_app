This is a Payment API server built on C++ Drogon Framework and PostgreSQL. Below are the available endpoints and their purposes.
/transaction
/transaction/{1}
/health/live
/health/ready
/setloglevel/{1}
/metrics

To compile the project, run 'cmake ..' from the payment_app/build folder.

To build a docker image, from the root of your project (where the Dockerfile is located), run:
docker build -t payment_app .
or to create an image on Docker Hub, replace adarshnanu with your Github account name.
docker build -t adarshnanu/payment_app:latest .

To run the image with your configuration:
docker run \
  -e PAYMENT_APP_CONFIG=/app/config/appsettings.json \
  -e PAYMENT_APP_LOG=/app/log/payment_app.log \
  payment_app

To run with Docker Hub Image:
docker run \
  -e PAYMENT_APP_CONFIG=/app/config/appsettings.json \
  -e PAYMENT_APP_LOG=/app/log/payment_app.log \
  adarshnanu/payment_app:latest

The project is Continuous Integartion enabled, and it will build the project when pushed to GitHub.

I am running PostgreSQL on Docker. RUn the below to start a container.
docker run -d \
  --name postgres-db \
  -e POSTGRES_USER=postgres \
  -e POSTGRES_PASSWORD=postgres123 \
  -e POSTGRES_DB=paymentdb \
  -p 5432:5432 \
  postgres:16

Run 'docker ps' to make sure it is running.

To connect to the container to check the database, use the below command,
docker exec -it postgres-db psql -U postgres -d paymentdb

The below environment variables need to be set for the applictaion to work. You can find these contents in appsettings.json
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

