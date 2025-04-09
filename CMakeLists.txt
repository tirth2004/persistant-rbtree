cmake_minimum_required(VERSION 3.10)
project(KeyValueDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files for server
set(SERVER_SOURCES
    src/servercorrect.cpp
    main.cpp
    src/watch_manager.cpp
)

# Create server executable
add_executable(kvdb ${SERVER_SOURCES})

# Link against required libraries for server
target_link_libraries(kvdb pthread)

# Add client executable
add_executable(kvdb_client client.cpp)

# Link against required libraries for client
target_link_libraries(kvdb_client) 

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(treap_tests test/treap_tests.cpp)
target_link_libraries(treap_tests gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(treap_tests)

add_executable(server_tests test/server_tests.cpp)
target_link_libraries(server_tests gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(server_tests)