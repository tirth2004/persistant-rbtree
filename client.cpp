#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <atomic>

std::atomic<bool> running(true);
int clientNumber = 0;

void receiveMessages(int clientSocket) {
    char buffer[1024];
    while (running) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            std::cerr << "Disconnected from server" << std::endl;
            running = false;
            break;
        }
        
        buffer[bytesRead] = '\0';
        std::string response(buffer);
        // Check if this is a welcome message with client number
        // if (response.substr(0, 7) == "WELCOME") {
        //     // clientNumber = std::stoi(response.substr(8));
        //     // std::cout << "You are client #" << clientNumber << std::endl;
        //     continue;
        // }
        // Check if this is a notification
        if (response.substr(0, 12) == "NOTIFICATION") {
            std::cout << "\n[NOTIFICATION] " << response.substr(13);
        } else {
            std::cout << response;
        }
        std::cout << "> "; // Always reprint the prompt
        std::cout.flush();
        
    }
}

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
    
    // Start a thread to receive messages
    std::thread receiveThread(receiveMessages, clientSocket);
    
    // Command loop
    std::string command;
    while (running) { 
        std::cout << "> ";
        std::getline(std::cin, command);
        // command.push_back('\n');
        if (command == "quit" || command == "exit") {
            running = false;
            break;
        }
        
        // Send command
        if (send(clientSocket, command.c_str(), command.length(), 0) < 0) {
            std::cerr << "Error sending command" << std::endl;
            running = false;
            break;
        }
    }
    
    // Clean up
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
    close(clientSocket);
    return 0;
}
