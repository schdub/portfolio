#include "writer.hpp"

#include <iostream>
#include <cassert>

// /////////////////////////////////////////////////// //

constexpr char LOG_NAME[] = "log.txt";
constexpr unsigned FLUSH_PERIOD_SECS = 5;

// /////////////////////////////////////////////////// //

Writer::Writer() {
    mLogFile.open(LOG_NAME);
    if (!mLogFile.is_open()) {
        std::cerr << "can't open log.txt" << std::endl;
        throw std::runtime_error("can't open log.txt");
    }
    mFlushTime = std::chrono::steady_clock::now();
    Start(1);
}

Writer::~Writer() {
    Stop();
    mLogFile.close();
}

void Writer::ServeJob(std::string data) {
    // проверим, открыт ли файл для чтения?
    if (!mLogFile.is_open()) {
        std::cerr << "reopen log.txt" << std::endl;
        mLogFile.open(LOG_NAME);
    }
    // запишем полученные данные в файл
    assert(mLogFile.is_open());
    mLogFile.write(data.data(), data.size());
    // проверим как давно не сбрасывали данные
    auto now = std::chrono::steady_clock::now();
    auto dur_count = std::chrono::duration_cast<std::chrono::seconds>(now - mFlushTime).count();
    if (dur_count >= FLUSH_PERIOD_SECS) {
        mFlushTime = now;
        mLogFile.flush();
    }
}
