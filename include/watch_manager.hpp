#ifndef WATCH_MANAGER_HPP
#define WATCH_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>

namespace kvdb {

// watch operation types
enum class WatchOperation {
    SET,
    DEL,
    EDIT,
    ALL
};

// hash function for WatchOperation
struct WatchOperationHash {
    std::size_t operator()(const WatchOperation& op) const {
        return static_cast<std::size_t>(op);
    }
};

// key for our hash map (key + operation)
struct WatchKey {
    std::string key;
    WatchOperation operation;
    
    bool operator==(const WatchKey& other) const {
        return key == other.key && operation == other.operation;
    }
};

// Hash function for WatchKey
struct WatchKeyHash {
    std::size_t operator()(const WatchKey& wk) const {
        return std::hash<std::string>()(wk.key) ^ 
               (static_cast<std::size_t>(wk.operation) << 1);
    }
};

// Notification structure
struct Notification {
    int clientSocket;
    std::string message;
};

class WatchManager {
public:
    WatchManager();
    ~WatchManager();

    // Watch registration methods - O(1) operations
    void addWatch(int clientSocket, const std::string& key, WatchOperation operation);
    void removeWatch(int clientSocket, const std::string& key, WatchOperation operation);
    void removeAllWatches(int clientSocket);
    
    // Notification methods - Asynchronous
    void notifyEvent(const std::string& key, WatchOperation operation, const std::string& value);
    
    // Start/stop notification thread
    void start();
    void stop();

private:
    // Fast lookup hash maps - O(1) access time
    std::mutex watchMutex;
    
    // Key+Operation -> Set of client sockets (for fast notification lookup)
    std::unordered_map<WatchKey, std::unordered_set<int>, WatchKeyHash> watchIndex;
    
    // Client socket -> Set of keys+operations (for fast client cleanup)
    std::unordered_map<int, std::unordered_set<WatchKey, WatchKeyHash>> clientIndex;
    
    // Notification queue for asynchronous processing
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::queue<Notification> notificationQueue;
    
    // Notification thread for asynchronous delivery
    std::thread notificationThread;
    std::atomic<bool> running;
    
    // Notification thread function
    void notificationLoop();
};

} // namespace kvdb

#endif // WATCH_MANAGER_HPP
