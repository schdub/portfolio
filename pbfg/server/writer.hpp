#pragma once

#include <op/pool.hpp>
#include <fstream>
#include <string>
#include <chrono>

class Writer : public op::WorkerPool<std::string> {
    std::chrono::steady_clock::time_point mFlushTime;
    std::ofstream mLogFile;
public:
    Writer();
    ~Writer();
    void ServeJob(std::string data) override;
};
