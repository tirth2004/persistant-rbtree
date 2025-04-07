# Key-Value Database with Snapshot and Restore

A simple key-value database system with a Red-Black tree implementation and a server-client architecture.

## Features

- Non-persistent Red-Black tree for efficient key-value storage
- Server-client architecture for remote access
- Basic operations: GET, SET, DEL
- Thread-safe server implementation

## Building the Project

### Prerequisites

- CMake (version 3.10 or higher)
- C++17 compatible compiler
- POSIX-compliant operating system (Linux, macOS)

### Build Steps

1. Clone the repository:

   ```
   git clone https://github.com/tirth2004/persistant-rbtree
   cd persistant-rbtree
   ```

2. Create a build directory and navigate to it:

   ```
   mkdir build
   cd build
   ```

3. Generate the build files:

   ```
   cmake ..
   ```

4. Build the project:
   ```
   make
   ```

## Running the Server

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
./kvdb_client 127.0.0.1 9000
```

## Available Commands

- `SET <key> <value>`: Set a key-value pair
- `GET <key>`: Get the value for a key
- `DEL <key>`: Delete a key-value pair
- `quit` or `exit`: Exit the client

## Example Usage

```
> SET user1 John
OK
> GET user1
OK John
> SET user1 Jane
OK
> GET user1
OK Jane
> DEL user1
OK
> GET user1
ERROR Key not found
> quit
```

## Future Enhancements

- Persistent Red-Black tree implementation
- Snapshot and restore functionality
- Complex value types
- Transaction support
- Replication and sharding
