Docker can be start like this. The parameters will change based on your environment. 
This will return a psql prompt where you can run SQL to check the data.
docker exec -it -e PGPASSWORD=postgres123 postgres-db psql -U postgres

The below environment variables need to be set for the applictaion to work.
export PAYMENT_APP_CONFIG=/path/to/appsettings.json
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

export PAYMENT_APP_LOG=/path/to/payment_app.log

