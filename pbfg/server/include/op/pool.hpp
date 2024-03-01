#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

namespace op {

template <typename T>
class WorkerPool {
    bool mTerminate = false;          // завершаемся?
    std::mutex mQueueMutex;           // мьюткс, защищающий очередь
    std::condition_variable mMutexCV;
    std::vector<std::thread> mThreads;
    std::queue<T> mJobs;

    void Loop();

protected:
    virtual void ServeJob(T job) = 0;

public:
    WorkerPool() {};
    virtual ~WorkerPool() {};

    void Start(unsigned int num_threads);
    void QueueJob(T job);
    void Stop();
    bool busy();
}; // WorkerPool

template <class T>
void WorkerPool<T>::Start(unsigned int num_threads) {
    // Max # of threads the system supports
    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
    }
    for (uint32_t ii = 0; ii < num_threads; ++ii) {
        mThreads.emplace_back(std::thread(&WorkerPool::Loop, this));
    }
}

template <class T>
void WorkerPool<T>::Loop() {
    for (;;) {
        T job; {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mMutexCV.wait(lock, [this] {
                return !mJobs.empty() || mTerminate;
            });
            if (mTerminate) {
                return;
            }
            job = mJobs.front();
            mJobs.pop();
        }
        ServeJob(std::move(job));
    }
}

template <class T>
void WorkerPool<T>::QueueJob(T sd) {
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mJobs.push(sd);
    }
    mMutexCV.notify_one();
}

template <class T>
bool WorkerPool<T>::busy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        poolbusy = !mJobs.empty();
    }
    return poolbusy;
}

template <class T>
void WorkerPool<T>::Stop() {
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mTerminate = true;
    }
    mMutexCV.notify_all();
    for (std::thread& active_thread : mThreads) {
        active_thread.join();
    }
    mThreads.clear();
}

} // namespace op
