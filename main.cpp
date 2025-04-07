#include "include/server.hpp"
#include <iostream>
#include <csignal>

kvdb::Server* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    if (g_server) {
        g_server->stop();
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
    
    // Create server
    kvdb::Server server(host, port);
    g_server = &server;
    
    // Register signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Start server
    server.start();
    
    // Wait for server to stop
    while (server.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "Server stopped." << std::endl;
    return 0;
}
