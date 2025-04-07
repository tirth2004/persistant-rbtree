#ifndef SERVER_HPP
#define SERVER_HPP

#include "rbtree.hpp"
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

namespace kvdb {

class Server {
public:
    Server(const std::string& host = "127.0.0.1", int port = 8080);
    ~Server();

    // Start the server
    void start();
    
    // Stop the server
    void stop();
    
    // Check if server is running
    bool isRunning() const;

private:
    std::string host;
    int port;
    std::atomic<bool> running;
    std::thread serverThread;
    std::vector<std::thread> clientThreads;
    std::mutex clientsMutex;
    
    // The key-value store
    RedBlackTree<std::string, std::string> store;
    
    // Server loop
    void serverLoop();
    
    // Handle client connection
    void handleClient(int clientSocket);
    
    // Process command
    std::string processCommand(const std::string& command);
    
    // Parse command
    struct Command {
        std::string operation;
        std::string key;
        std::string value;
    };
    Command parseCommand(const std::string& commandStr);
};

} // namespace kvdb

#endif // SERVER_HPP 