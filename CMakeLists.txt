cmake_minimum_required(VERSION 3.10)
project(KeyValueDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files for server
set(SERVER_SOURCES
    src/server.cpp
    main.cpp
)

# Create server executable
add_executable(kvdb ${SERVER_SOURCES})

# Link against required libraries for server
target_link_libraries(kvdb pthread)

# Add client executable
add_executable(kvdb_client client.cpp)

# Link against required libraries for client
target_link_libraries(kvdb_client) 