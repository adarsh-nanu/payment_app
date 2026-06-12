cmake_minimum_required(VERSION 3.5)
project(payment_app_test CXX)


add_executable(${PROJECT_NAME}
    test_main.cc
    ${TEST_SRC_FILES}
)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

ParseAndAddDrogonTests(${PROJECT_NAME})
