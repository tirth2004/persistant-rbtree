#ifndef SERVER_HPP
#define SERVER_HPP

#include "PersistentTreap.hpp"
#include "watch_manager.hpp"
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>

namespace kvdb {

class Server {
public:
    Server(const std::string& host = "127.0.0.1", int port = 8080);
    ~Server();

    void start();
    void stop();
    bool isRunning() const;

private:
    std::atomic<int> clientCounter{0};
    std::string host;
    int port;
    std::atomic<bool> running;
    std::thread serverThread;
    std::vector<std::thread> clientThreads;
    std::mutex clientsMutex;

    // The key-value store
    Treap<std::string, std::string> store;
    
    // Watch manager for event notifications
    WatchManager watchManager;

    void serverLoop();
    void handleClient(int clientSocket);
    std::string processCommand(const std::string& command, int clientSocket);

    struct Command {
        std::string operation;
        std::string key;
        std::string value;
        int version;
        int clientSocket;
    };
    Command parseCommand(const std::string& commandStr);
};

} // namespace kvdb

#endif // SERVER_HPP
