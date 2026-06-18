#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include "../service/TransactionService.h"
#include "../util/ConfigManager.h"

DROGON_TEST(SingleTonTest)
{
    // Add your tests here
    TransactionService &obj1 = TransactionService::getInstance();
    TransactionService &obj2 = TransactionService::getInstance();
    //CHECK( &obj1 != &obj2 );
    REQUIRE( &obj1 == &obj2 );
}

DROGON_TEST(MsgQueEnqueue)
{
    // Add your tests here
    TransactionService &obj1 = TransactionService::getInstance();
    
    TransactionJob TJobj{"test_0001", "adarsh nanu", 15.66};
    int current_count = obj1.getMessagesInQueueCount();
    REQUIRE_THROWS(obj1.enqueue(TJobj));
    ConfigManager& cfg = ConfigManager::getInstance();
    obj1.setMessagesInQueueCount( cfg.getInt("maxMessagesInQueue") );
    obj1.enqueue(TJobj);
    CHECK( obj1.getMessagesInQueueCount() == ( current_count+1 ) );

}

DROGON_TEST(BasicTest1)
{
    // Add your tests here
    CHECK( ( 2+2 ) == 4 );
    /*CHECK( ( 2+2 ) == 41 );
    REQUIRE( (2+2) == 4);
    REQUIRE( (2+2) == 3);
    CHECK( ( 3+2 ) == 5 );*/

}
int main(int argc, char** argv) 
{
    using namespace drogon;

    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    // Start the main loop on another thread
    std::thread thr([&]() {
        // Queues the promise to be fulfilled after starting the loop
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    // The future is only satisfied after the event loop started
    f1.get();
    int status = test::run(argc, argv);

    // Ask the event loop to shutdown and wait
    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}
