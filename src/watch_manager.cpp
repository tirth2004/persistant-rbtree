#include "../include/watch_manager.hpp"
#include <algorithm>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

namespace kvdb {

WatchManager::WatchManager() : running(false) {}

WatchManager::~WatchManager() {
    stop();
}

void WatchManager::start() {
    if (!running.exchange(true)) {
        notificationThread = std::thread(&WatchManager::notificationLoop, this);
    }
}

void WatchManager::stop() {
    if (running.exchange(false)) {
        queueCV.notify_one();
        if (notificationThread.joinable()) {
            notificationThread.join();
        }
    }
}

// O(1) operation - add watch with hash-based indexing
void WatchManager::addWatch(int clientSocket, const std::string& key, WatchOperation operation) {
    std::lock_guard<std::mutex> lock(watchMutex);
    
    // Create the watch key
    WatchKey watchKey{key, operation};
    
    // Add to watch index (key+operation -> clients)
    watchIndex[watchKey].insert(clientSocket);
    
    // Add to client index (client -> keys+operations)
    clientIndex[clientSocket].insert(watchKey);
}

// O(1) operation - remove specific watch
void WatchManager::removeWatch(int clientSocket, const std::string& key, WatchOperation operation) {
    std::lock_guard<std::mutex> lock(watchMutex);
    
    WatchKey watchKey{key, operation};
    
    // Remove from watch index
    auto watchIt = watchIndex.find(watchKey);
    if (watchIt != watchIndex.end()) {
        watchIt->second.erase(clientSocket);
        if (watchIt->second.empty()) {
            watchIndex.erase(watchIt);
        }
    }
    
    // Remove from client index
    auto clientIt = clientIndex.find(clientSocket);
    if (clientIt != clientIndex.end()) {
        clientIt->second.erase(watchKey);
        if (clientIt->second.empty()) {
            clientIndex.erase(clientIt);
        }
    }
}

// O(n) operation where n is the number of watches for this client
void WatchManager::removeAllWatches(int clientSocket) {
    std::lock_guard<std::mutex> lock(watchMutex);
    
    auto clientIt = clientIndex.find(clientSocket);
    if (clientIt != clientIndex.end()) {
        // For each key+operation this client is watching
        for (const auto& watchKey : clientIt->second) {
            // Remove client from the watch index
            auto watchIt = watchIndex.find(watchKey);
            if (watchIt != watchIndex.end()) {
                watchIt->second.erase(clientSocket);
                if (watchIt->second.empty()) {
                    watchIndex.erase(watchIt);
                }
            }
        }
        // Remove all client watches
        clientIndex.erase(clientIt);
    }
}

// Asynchronous notification - O(1) lookup, O(n) queue where n is number of clients to notify
void WatchManager::notifyEvent(const std::string& key, WatchOperation operation, const std::string& value) {
    // Format the notification message
    std::string opStr;
    switch (operation) {
        case WatchOperation::SET: opStr = "SET"; break;
        case WatchOperation::DEL: opStr = "DEL"; break;
        case WatchOperation::EDIT: opStr = "EDIT"; break;
        default: opStr = "UNKNOWN"; break;
    }
    
    std::string notificationMsg = "NOTIFICATION " + opStr + " " + key;
    if (operation != WatchOperation::DEL) {
        notificationMsg += " " + value;
    }
    notificationMsg += "\n";
    
    // Get clients to notify with O(1) lookup
    std::unordered_set<int> clientsToNotify;
    {
        std::lock_guard<std::mutex> lock(watchMutex);
        
        // Check specific operation watchers
        WatchKey specificKey{key, operation};
        auto specificIt = watchIndex.find(specificKey);
        if (specificIt != watchIndex.end()) {
            clientsToNotify.insert(specificIt->second.begin(), specificIt->second.end());
        }
        
        // Check ALL operation watchers
        WatchKey allKey{key, WatchOperation::ALL};
        auto allIt = watchIndex.find(allKey);
        if (allIt != watchIndex.end()) {
            clientsToNotify.insert(allIt->second.begin(), allIt->second.end());
        }
    }
    
    // Queue notifications asynchronously
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (int clientSocket : clientsToNotify) {
            notificationQueue.push({clientSocket, notificationMsg});
        }
    }
    
    // Signal notification thread
    queueCV.notify_one();
}

// Asynchronous notification delivery thread
void WatchManager::notificationLoop() {
    while (running) {
        std::vector<Notification> batch;
        
        // Wait for notifications and batch them
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [this] { 
                return !notificationQueue.empty() || !running; 
            });
            
            if (!running && notificationQueue.empty()) {
                break;
            }
            
            // Process notifications in batches for efficiency
            while (!notificationQueue.empty() && batch.size() < 100) { // Batch size limit
                batch.push_back(notificationQueue.front());
                notificationQueue.pop();
            }
        }
        
        // Send notifications (outside of lock)
        for (const auto& notification : batch) {
            if (notification.clientSocket > 0) {
                // Try to send, ignore errors (client might have disconnected)
                send(notification.clientSocket, notification.message.c_str(), 
                     notification.message.length(), MSG_NOSIGNAL);
            }
        }
    }
}

}
