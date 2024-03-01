#pragma once

#include <string>

#include <op/net.hpp>

#include "workers.hpp"
#include "writer.hpp"

class Server : public op::TCPServer {
    Workers mWorkersPool;
    Writer mWriterPool;
    void incomingConnection(SOCKET sd, sockaddr * sa, socklen_t sa_len) override;
public:
    Server(unsigned port);
    ~Server();
    void appendData(std::string data);
};
