#include "../include/server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace kvdb {

Server::Server(const std::string& host, int port)
    : host(host), port(port), running(false) {
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (running) {
        return;
    }
    
    running = true;
    serverThread = std::thread(&Server::serverLoop, this);
}

void Server::stop() {
    if (!running) {
        return;
    }
    
    running = false;
    
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    // Wait for all client threads to finish
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    clientThreads.clear();
}

bool Server::isRunning() const {
    return running;
}

void Server::serverLoop() {
    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        running = false;
        return;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        close(serverSocket);
        running = false;
        return;
    }
    
    // Bind socket
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(serverSocket);
        running = false;
        return;
    }
    
    // Listen for connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        running = false;
        return;
    }
    
    std::cout << "Server listening on " << host << ":" << port << std::endl;
    
    // Accept connections
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (running) {
                std::cerr << "Error accepting connection" << std::endl;
            }
            continue;
        }
        
        // Create a new thread to handle the client
        std::lock_guard<std::mutex> lock(clientsMutex);
        clientThreads.emplace_back(&Server::handleClient, this, clientSocket);
    }
    
    close(serverSocket);
}

void Server::handleClient(int clientSocket) {
    char buffer[1024];
    
    while (running) {
        // Receive command
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            break;
        }
        
        buffer[bytesRead] = '\0';
        std::string command(buffer);
        
        // Process command
        std::string response = processCommand(command);
        
        // Send response
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    
    close(clientSocket);
}

std::string Server::processCommand(const std::string& command) {
    Command cmd = parseCommand(command);
    
    if (cmd.operation == "GET") {
        auto value = store.get(cmd.key);
        if (value) {
            return "OK " + *value + "\n";
        } else {
            return "ERROR Key not found\n";
        }
    } else if (cmd.operation == "SET") {
        if (store.insert(cmd.key, cmd.value)) {
            return "OK\n";
        } else {
            return "ERROR Failed to set key\n";
        }
    } else if (cmd.operation == "DEL") {
        if (store.remove(cmd.key)) {
            return "OK\n";
        } else {
            return "ERROR Key not found\n";
        }
    } else {
        return "ERROR Unknown command\n";
    }
}

Server::Command Server::parseCommand(const std::string& commandStr) {
    Command cmd;
    std::istringstream iss(commandStr);
    std::string token;
    
    // Parse operation
    if (std::getline(iss, token, ' ')) {
        cmd.operation = token;
    }
    
    // Parse key
    if (std::getline(iss, token, ' ')) {
        cmd.key = token;
    }
    
    // Parse value (if any)
    if (std::getline(iss, token)) {
        cmd.value = token;
    }
    
    return cmd;
}

} // namespace kvdb 