#ifndef SERVER_HPP
#define SERVER_HPP

#include "PersistentTreap.hpp"
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
    std::string host;
    int port;
    std::atomic<bool> running;
    std::thread serverThread;
    std::vector<std::thread> clientThreads;
    std::mutex clientsMutex;

    // The key-value store
    Treap<std::string, std::string> store;
    // Remove this line as we'll use the global versions vector from PersistentTreap.hpp
    // std::vector<Treap<std::string, std::string>> versions;

    void serverLoop();
    void handleClient(int clientSocket);
    std::string processCommand(const std::string& command);

    struct Command {
        std::string operation;
        std::string key;
        std::string value;
        int version;
    };
    Command parseCommand(const std::string& commandStr);
};

} // namespace kvdb

#endif // SERVER_HPP
