#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    // Default host and port
    std::string host = "127.0.0.1";
    int port = 8080;
    
    // Parse command line arguments
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }
    
    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }
    
    // Connect to server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
    serverAddr.sin_port = htons(port);
    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        close(clientSocket);
        return 1;
    }
    
    std::cout << "Connected to server at " << host << ":" << port << std::endl;
    
    // Command loop
    std::string command;
    char buffer[1024];
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, command);
        // command.push_back('\n');
        if (command == "quit" || command == "exit") {
            break;
        }
        
        // Send command
        if (send(clientSocket, command.c_str(), command.length(), 0) < 0) {
            std::cerr << "Error sending command" << std::endl;
            break;
        }
        
        // Receive response
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            std::cerr << "Error receiving response" << std::endl;
            break;
        }
        
        buffer[bytesRead] = '\0';
        std::cout << buffer;
    }
    
    close(clientSocket);
    return 0;
} 