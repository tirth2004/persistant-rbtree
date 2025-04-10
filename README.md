# Key-Value Database with Snapshot and Restore

A simple key-value database system with a Persistent Treap implementation and a server-client architecture.

## Features

- Persistent Treap for efficient key-value storage with O(log n) operations
- Version control with snapshots and rollback capability
- Server-client architecture for remote access
- Basic operations: GET, SET, DEL, EDIT
- Advanced operations: SNAPSHOT, VGET (versioned get), CHANGE
- Watch/Notify system for real-time updates
- Thread-safe server implementation with epoll-based multiplexing

## Building the Project

### Prerequisites

- CMake (version 3.10 or higher)
- C++17 compatible compiler
- POSIX-compliant operating system (Linux, macOS)
- Docker and Docker Compose (for containerized deployment)

### Build Steps

1. Clone the repository:

   ```
   git clone https://github.com/tirth2004/persistant-rbtree
   cd persistant-rbtree
   ```

2. Using Docker (recommended):

   ```
   # First, build the Docker images
   docker-compose build

   # Then start the server
   docker-compose up server
   ```

3. Or build locally:

   ```
   # Build everything
   mkdir build
   cd build
   cmake ..
   make
   ```

4. If the full build fails due to problematic libraries:

   ```
   # First, build and start the server in Docker
   docker-compose build
   docker-compose up server

   # Then, in a new terminal, build specific components
   mkdir build_client
   cd build_client
   cmake ..

   # Build only the client and tests
   make kvdb_client
   make server_tests
   make client_tests
   ```

## Running Tests

### Important Notes

- **Server Requirements**: The `server_tests` require a running server instance. Make sure to start the server before running these tests.
- **Server State**: All operations performed by the tests will be executed on the server. Always restart the server before running tests again or using it with clients to ensure a clean state.
- **Test Isolation**: Each test run modifies the server's state, so it's recommended to restart the server between test runs.

### Running Tests Locally

1. Start the server first:

   ```
   # Using Docker (recommended)
   docker-compose up server

   # Or locally
   ./kvdb 0.0.0.0 8080
   ```

2. In a new terminal, run the tests:

   ```
   # Run all tests
   ./treap_tests
   ./server_tests

   # Or run specific tests
   ./server_tests --gtest_filter=ServerTest.TestInsert
   ```

### Running Tests in Docker

```
# Run server tests
docker-compose up server_tests

# Run treap tests
docker-compose up treap_tests
```

## Running the Server

### Using Docker (recommended)

To start the server in Docker:

```
# First build the images
docker-compose build

# Then start the server
docker-compose up server
```

The server will be available at 0.0.0.0:8080 by default.

### Running Locally

To start the server with default settings (localhost:8080):

```
./kvdb
```

To specify a different host and port:

```
./kvdb <host> <port>
```

Example:

```
./kvdb 0.0.0.0 9000
```

## Using the Client

### Running Locally

To connect to the server with default settings (localhost:8080):

```
./kvdb_client
```

To specify a different host and port:

```
./kvdb_client <host> <port>
```

Example:

```
./kvdb_client 127.0.0.1 8080
```

## Available Commands

Basic Operations:

- `SET <key> <value>`: Set a key-value pair
- `GET <key>`: Get the value for a key
- `DEL <key>`: Delete a key-value pair
- `EDIT <key> <value>`: Edit an existing key's value

Version Control:

- `SNAPSHOT`: Create a new version snapshot
- `VGET <version> <key>`: Get value from a specific version
- `CHANGE <version>` : revert back to specified version

Watch/Notify:

- `WATCH <key> <operation>`: Watch a key for specific operations (SET/DEL/EDIT/ALL)
- `UNWATCH <key> <operation>`: Stop watching a key
- `UNWATCH`: Stop all watches

Store/Load:

- `STORE <file name>` : Store the current DB with all its SNAPSHOTS to the specified file
- `VSTORE <file name>` : Store the current DB only without SNAPSHOTS to the specified file
- `LOAD <file name>` : Load the DB with it's SNAPSHOTS from the specified file
- `VLOAD <file name>` : Load the DB only from the specified file.

Other:

- `quit` or `exit`: Exit the client

## Example Usage

```
> SET user1 John
OK
> GET user1
OK John
> SNAPSHOT
OK Snapshot created, version 0
> EDIT user1 Jane
OK
> GET user1
OK Jane
> VGET 0 user1
OK John
> WATCH user1 ALL (In new client)
OK Watching user1 for ALL operations
> EDIT user1 Alice (In original client)
OK (In original client)
[NOTIFICATION] EDIT user1 Alice (In new client)
```
