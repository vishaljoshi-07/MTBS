#include "../include/thread_manager.h"
#include <iostream>
#include <algorithm>
#include <chrono>

// ============================================================================
// THREAD POOL IMPLEMENTATION
// ============================================================================

ThreadPool::Task::Task(const std::function<void()>& func, const std::string& desc, int prio)
    : function(func), description(desc), priority(prio) {
}

ThreadPool::ThreadPool(size_t threadCount) 
    : running(false), activeThreads(0) {
    
    // Create worker threads
    for (size_t i = 0; i < threadCount; ++i) {
        createWorker();
    }
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {
    std::lock_guard<std::mutex> lock(poolMutex);
    if (!running) {
        running = true;
        condition.notify_all();
    }
}

void ThreadPool::stop() {
    {
        std::lock_guard<std::mutex> lock(poolMutex);
        running = false;
    }
    condition.notify_all();
    
    // Wait for all worker threads to finish
    for (auto& worker : workers) {
        if (worker && worker->joinable()) {
            worker->join();
        }
    }
    workers.clear();
}

bool ThreadPool::submitTask(const std::function<void()>& task, const std::string& description) {
    std::lock_guard<std::mutex> lock(poolMutex);
    
    if (!running) {
        return false;
    }
    
    // Create task with default priority
    Task newTask(task, description, 0);
    taskQueue.push(newTask);
    
    // Notify a waiting worker thread
    condition.notify_one();
    return true;
}

size_t ThreadPool::getActiveThreadCount() const {
    return activeThreads.load();
}

size_t ThreadPool::getQueueSize() const {
    std::lock_guard<std::mutex> lock(poolMutex);
    return taskQueue.size();
}

bool ThreadPool::isRunning() const {
    return running.load();
}

void ThreadPool::createWorker() {
    auto worker = std::make_unique<std::thread>([this]() {
        workerFunction();
    });
    workers.push_back(std::move(worker));
}

void ThreadPool::workerFunction() {
    while (running) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(poolMutex);
            
            // Wait for a task or stop signal
            condition.wait(lock, [this]() {
                return !running || !taskQueue.empty();
            });
            
            if (!running && taskQueue.empty()) {
                break;
            }
            
            if (!taskQueue.empty()) {
                task = taskQueue.top();
                taskQueue.pop();
                activeThreads++;
            }
        }
        
        // Execute the task
        if (task.function) {
            try {
                task.function();
            } catch (const std::exception& e) {
                std::cerr << "Error in worker thread: " << e.what() << std::endl;
            }
        }
        
        activeThreads--;
    }
}

// ============================================================================
// WORK QUEUE IMPLEMENTATION
// ============================================================================

WorkQueue::WorkItem::WorkItem(const std::function<void()>& w, const std::string& desc)
    : work(w), description(desc), timestamp(std::chrono::system_clock::now()) {
}

WorkQueue::WorkQueue(size_t maxSize) : maxSize(maxSize) {
}

bool WorkQueue::enqueue(const WorkItem& item) {
    std::unique_lock<std::mutex> lock(queueMutex);
    
    // Wait if queue is full
    notFull.wait(lock, [this]() { return queue.size() < maxSize; });
    
    queue.push(item);
    
    // Notify waiting consumers
    notEmpty.notify_one();
    return true;
}

bool WorkQueue::dequeue(WorkItem& item) {
    std::unique_lock<std::mutex> lock(queueMutex);
    
    // Wait if queue is empty
    notEmpty.wait(lock, [this]() { return !queue.empty(); });
    
    item = queue.front();
    queue.pop();
    
    // Notify waiting producers
    notFull.notify_one();
    return true;
}

size_t WorkQueue::size() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return queue.size();
}

bool WorkQueue::empty() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return queue.empty();
}

// ============================================================================
// THREAD MONITOR IMPLEMENTATION
// ============================================================================

void ThreadMonitor::registerThread(const std::thread::id& id, const std::string& name) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    
    ThreadInfo info;
    info.id = id;
    info.name = name;
    info.status = "RUNNING";
    info.startTime = std::chrono::system_clock::now();
    
    threads[id] = info;
}

void ThreadMonitor::updateThreadStatus(const std::thread::id& id, const std::string& status) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    
    auto it = threads.find(id);
    if (it != threads.end()) {
        it->second.status = status;
    }
}

std::vector<ThreadMonitor::ThreadInfo> ThreadMonitor::getAllThreads() const {
    std::lock_guard<std::mutex> lock(monitorMutex);
    
    std::vector<ThreadInfo> result;
    result.reserve(threads.size());
    
    for (const auto& pair : threads) {
        result.push_back(pair.second);
    }
    
    return result;
}

