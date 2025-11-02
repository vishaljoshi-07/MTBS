#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <memory>
#include <chrono>
#include <string>
#include <map>

/**
 * @brief Thread pool manager for handling concurrent banking operations
 */
class ThreadPool {
public:
    struct Task {
        std::function<void()> function;
        std::string description;
        int priority;
        
        Task() : function(nullptr), description(""), priority(0) {}
        Task(const std::function<void()>& func, const std::string& desc = "", int prio = 0);
        bool operator<(const Task& other) const { return priority < other.priority; }
    };
    
    explicit ThreadPool(size_t threadCount = 4);
    ~ThreadPool();
    
    void start();
    void stop();
    bool submitTask(const std::function<void()>& task, const std::string& description = "");
    
    size_t getActiveThreadCount() const;
    size_t getQueueSize() const;
    bool isRunning() const;

private:
    std::vector<std::unique_ptr<std::thread>> workers;
    std::priority_queue<Task> taskQueue;
    
    mutable std::mutex poolMutex;
    std::condition_variable condition;
    
    std::atomic<bool> running;
    std::atomic<size_t> activeThreads;
    
    void createWorker();
    void workerFunction();
};

/**
 * @brief Work queue for transaction processing
 */
class WorkQueue {
public:
    struct WorkItem {
        std::function<void()> work;
        std::string description;
        std::chrono::system_clock::time_point timestamp;
        
        WorkItem(const std::function<void()>& w, const std::string& desc);
    };
    
    explicit WorkQueue(size_t maxSize = 1000);
    
    bool enqueue(const WorkItem& item);
    bool dequeue(WorkItem& item);
    size_t size() const;
    bool empty() const;

private:
    std::queue<WorkItem> queue;
    mutable std::mutex queueMutex;
    std::condition_variable notEmpty;
    std::condition_variable notFull;
    size_t maxSize;
};

/**
 * @brief Thread monitoring utilities
 */
class ThreadMonitor {
public:
    struct ThreadInfo {
        std::thread::id id;
        std::string name;
        std::string status;
        std::chrono::system_clock::time_point startTime;
    };
    
    void registerThread(const std::thread::id& id, const std::string& name);
    void updateThreadStatus(const std::thread::id& id, const std::string& status);
    std::vector<ThreadInfo> getAllThreads() const;

private:
    mutable std::mutex monitorMutex;
    std::map<std::thread::id, ThreadInfo> threads;
};

#endif // THREAD_MANAGER_H
