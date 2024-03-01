#pragma once

#include <op/pool.hpp>
#include <op/net.hpp>

class Server;

class Workers : public op::WorkerPool<SOCKET> {
    Server * mServer;
public:
    Workers(Server * pServer);
    ~Workers();
    void ServeJob(SOCKET sd) override;
};
