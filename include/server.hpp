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

    void start();           // start server
    void stop();            // stop server
    bool isRunning() const; // check if server is running

private:
    std::atomic<int> clientCounter{0};          // shared variable hence atomic for thread safety
    std::string host;
    int port;
    std::atomic<bool> running;                  // shared variable hence atomic for thread safety
    std::thread serverThread;
    std::vector<std::thread> clientThreads;
    std::mutex clientsMutex;

    // The key-value store
    Treap<std::string, std::string> store;      // our main DS persistent treap (this version assumes value as a string which can be udated later on for object for more flexitbity [thanks to me for creating template classes])
    
    // Watch manager for event notifications
    WatchManager watchManager;

    void serverLoop();                          // ?
    void handleClient(int clientSocket);        // ?
    std::string processCommand(const std::string& command, int clientSocket);           //  execute the command on treap

    struct Command {                            // This structre will store our command which will later be fed to Treap orz
        std::string operation;
        std::string key;
        std::string value;
        int version;
        int clientSocket;
    };
    Command parseCommand(const std::string& commandStr);    // parse the command
};

}

#endif
