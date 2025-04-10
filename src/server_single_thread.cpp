/* All client socket connections are handled by the same server thread, which prevents
 the overhead associated with creating and managing many threads. This avoids performance
 issues like excessive context switching and also leads to lower memory usage since each thread
 will otherwise hold its own memory stack. Combined with epoll, this allows us to make a
 highly scalable server that can manage multiple clients. Since only one operation runs
 at a time in the single thread, there are also no concurrency issues with read/write operations.*/
#include "../include/server.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unordered_map>
#include <cstring>

namespace kvdb {

// Define server class and its methods
Server::Server(const std::string& host, int port)
    : host(host), port(port), running(false) {
    watchManager.start();
    }

Server::~Server() {
    stop();
    watchManager.stop();
}

void Server::start() {
    if (running) return;
    running = true;
    serverThread = std::thread(&Server::serverLoop, this);
}

void Server::stop() {
    if (!running) return;
    running = false;
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

bool Server::isRunning() const {
    return running;
}

void Server::serverLoop() {
    versions<std::string, std::string>.clear();
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(serverSocket);
        return;
    }

    if (listen(serverSocket, SOMAXCONN) < 0) {
        std::cerr << "Error listening" << std::endl;
        close(serverSocket);
        return;
    }

    int epollFd = epoll_create1(0);
    if (epollFd < 0) {
        std::cerr << "Failed to create epoll file descriptor" << std::endl;
        close(serverSocket);
        return;
    }

    /* Set file descriptor to non-blocking mode so one socket does not hold/block the server.
     Prevents one socket from holding up the server in case of any I/O or network failure.*/
    auto makeNonBlocking = [](int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    };

    makeNonBlocking(serverSocket);

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event);

    std::unordered_map<int, std::string> partialBuffer;

    std::cout << "Server listening on " << host << ":" << port << std::endl;

    // Upto 64 clients are handled in one call to epoll_wait. Rest will be handled in the next call.
    const int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];

    // Start event loop
    while (running) {
        int n = epoll_wait(epollFd, events, MAX_EVENTS, 1000);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            
            // New client is trying to connect
            if (fd == serverSocket) {
                struct sockaddr_in clientAddr;
                socklen_t clientLen = sizeof(clientAddr);
                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
                if (clientSocket >= 0) {
                    makeNonBlocking(clientSocket);
                    struct epoll_event clientEvent;
                    clientEvent.events = EPOLLIN | EPOLLET;
                    clientEvent.data.fd = clientSocket;
                    epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent);

                    std::cerr << "New client connection accepted. Thread ID: " 
                              << std::this_thread::get_id() << std::endl;
                }

            // Existing client is sending new data
            } else {
                char buffer[1024];
                while (true) {
                    int bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytesRead < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
                            close(fd);
                            partialBuffer.erase(fd);
                        }
                        break;
                    } else if (bytesRead == 0) {
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        partialBuffer.erase(fd);
                        break;
                    } else {
                        buffer[bytesRead] = '\0';
                        std::string command(buffer); // take the whole chunk
                        std::string response = processCommand(command, fd);
                        send(fd, response.c_str(), response.length(), 0);
                    }
                }
            }
        }
    }

    close(serverSocket);
    close(epollFd);
}

// Read command and call appropriate functions.
std::string Server::processCommand(const std::string& command, int clientSocket) {
    Command cmd = parseCommand(command);
    cmd.clientSocket = clientSocket;
    
    if (cmd.operation == "WATCH") {
        WatchOperation op;
        if (cmd.value == "SET") {
            op = WatchOperation::SET;
        } else if (cmd.value == "DEL") {
            op = WatchOperation::DEL;
        } else if (cmd.value == "EDIT") {
            op = WatchOperation::EDIT;
        } else if (cmd.value == "ALL") {
            op = WatchOperation::ALL;
        } else {
            return "ERROR Invalid watch operation. Use SET, DEL, EDIT, or ALL\n";
        }
        
        watchManager.addWatch(clientSocket, cmd.key, op);
        return "OK Watching " + cmd.key + " for " + cmd.value + " operations\n";
    } 
    else if (cmd.operation == "UNWATCH") {
        if (cmd.key.empty()) {
            watchManager.removeAllWatches(clientSocket);
            return "OK Removed all watches\n";
        } else {
            WatchOperation op;
            if (cmd.value == "SET") {
                op = WatchOperation::SET;
            } else if (cmd.value == "DEL") {
                op = WatchOperation::DEL;
            } else if (cmd.value == "EDIT") {
                op = WatchOperation::EDIT;
            } else if (cmd.value == "ALL") {
                op = WatchOperation::ALL;
            } else {
                return "ERROR Invalid watch operation\n";
            }
            
            watchManager.removeWatch(clientSocket, cmd.key, op);
            return "OK Removed watch for " + cmd.key + "\n";
        }
    }
    else if (cmd.operation == "GET") {
        auto value = store.find(cmd.key);
        if (value.has_value()) {
            return "OK " + *value + "\n";
        } else {
            return "ERROR Key not found\n";
        }
    } 
    else if (cmd.operation == "SET") {
        auto existingValue = store.find(cmd.key);
        if (existingValue.has_value()) {
            return "ERROR Key already exists\n";  
        }
        store.insert(cmd.key, cmd.value);
        watchManager.notifyEvent(cmd.key, WatchOperation::SET, cmd.value);
        return "OK\n";
    }
    
    else if (cmd.operation == "DEL") {
        auto existingValue = store.find(cmd.key);
        if (existingValue.has_value()) {
            store.remove(cmd.key);
            watchManager.notifyEvent(cmd.key, WatchOperation::DEL, "");
            return "OK\n";
        } else {
            return "ERROR Key not found\n";  
        }
    }
    else if (cmd.operation == "EDIT") {
        auto existingValue = store.find(cmd.key);
        if (existingValue.has_value()) {
            store.edit(cmd.key, cmd.value);
            watchManager.notifyEvent(cmd.key, WatchOperation::EDIT, cmd.value);
            return "OK\n";
        } else {
            return "ERROR Key not found\n";  
        }
    }
    
    else if (cmd.operation == "SNAPSHOT") {
        snapshot<std::string, std::string>(store);
        return "OK Snapshot created, version " + 
               std::to_string(versions<std::string, std::string>.size() - 1) + "\n";
    }
    else if (cmd.operation == "VGET") {
        if (cmd.version >= 0 && cmd.version < versions<std::string, std::string>.size()) {
            auto rolledBackTreap = rollback<std::string, std::string>(cmd.version);
            auto value = rolledBackTreap.find(cmd.key);
            if (value.has_value()) {
                return "OK " + *value + "\n";
            } else {
                return "ERROR Key not found in version " + std::to_string(cmd.version) + "\n";
            }
        } else {
            return "ERROR Invalid version\n";
        }
    }
    else if(cmd.operation == "STORE")
    {
        ofstream os("../save/"+cmd.value);
        save<string, string>(os, store.root);
        os.close();
        return "DATABASE and SNAPSHOTS saved to " + cmd.value + "\n";
    }
    else if(cmd.operation == "VSTORE"){
        ofstream os("../save/"+cmd.value);
        store.save(os);
        os.close();
        return "DATABASE saved to " + cmd.value + "\n";
    }
    else if (cmd.operation == "LOAD")
    {
        ifstream is("../save/"+cmd.value);
        if(!is.is_open()){
            return "ERROR in opening " + cmd.value + "\n";
        }
        int root = load<string, string>(is);
        store = Treap<string, string>(root);
        is.close();
        return "DATABASE and SNAPSHOTS Loaded\n";
    }
    else if (cmd.operation == "VLOAD")
    {
        ifstream is("../save/"+cmd.value);
        if(!is.is_open()){
            return "ERROR in opening " + cmd.value;
        }
        store.load(is);
        is.close();
        return "DATABASE Loaded\n";
    }
    else if(cmd.operation == "CHANGE")
    {
        store = rollback<string, string>(cmd.version);
        return "CHANGE to version " + to_string(cmd.version) + "\n";
    }
    else {
        return "ERROR Unknown command\n";
    }
}

// Fill command struct according to command entered.
Server::Command Server::parseCommand(const std::string& commandStr) {
    Command cmd;
    cmd.version = -1;
    cmd.clientSocket = -1;
    
    std::istringstream iss(commandStr);
    std::string token;
    
    if (std::getline(iss, token, ' ')) {
        cmd.operation = (token);
    }
    
    if (cmd.operation == "VGET") {
        if (std::getline(iss, token, ' ')) {
            cmd.version = std::stoi(token);
        }
        if (std::getline(iss, token, ' ')) {
            cmd.key = token;
        }
    } 
    else if(cmd.operation == "STORE" || cmd.operation == "VSTORE" || cmd.operation == "LOAD" || cmd.operation == "VLOAD")
    {
        if(std :: getline(iss, token, ' '))
        {
            cmd.value = token;
        }
    }
    else if (cmd.operation == "CHANGE")
    {
        if(std :: getline(iss, token, ' '))
        {
            cmd.version = stoi(token);
        }
    }
    
    else {
        if (std::getline(iss, token, ' ')) {
            cmd.key = token;
        }
        if (std::getline(iss, token)) {
            cmd.value = token;
        }
    }
    
    return cmd;
}

}
