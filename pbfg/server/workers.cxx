#include "workers.hpp"

#include <iostream>
#include <thread>

#include <op/net.hpp>

#include "server.hpp"

Workers::Workers(Server *pServer)
    : mServer(pServer)
{
    Start(std::thread::hardware_concurrency());
}

Workers::~Workers() {
    Stop();
}

void Workers::ServeJob(SOCKET sd) {
    op::TCPSocket socket(sd);
    if (!socket.isOk()) {
        std::cerr << "bad socket " << sd << std::endl;
        return;
    }

    const size_t increment_value = 1024;
    const size_t max_count = 1024 * 10;
    std::string data;
    data.resize(increment_value);
    int total = 0;

    for (;;) {
        int count = socket.recvthis( data.data() + total, data.size() - total );
        if (count == 0) {
            // данные не оканчиваются символом \n
            // игнорируем их
            data.resize(0);
            break;
        }
        total += count;
        if (data[total-1] == '\n') {
            // "хорошие" данные
            data.resize(total);
            break;
        }
        if (total == data.size()) {
            if (total >= max_count) {
                // слишком много данных
                // обрезаем их
                std::cerr << "date size exceeds max_count!!!" << std::endl;
                data.resize(total);
                data[total-1] = '\n';
                break;
            }
            data.resize(data.size() + increment_value);
        }
    }
    // есть данные?
    if (!data.empty()) {
        mServer->appendData(std::move(data));
    }
}
