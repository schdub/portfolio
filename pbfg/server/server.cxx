#include "server.hpp"

#include <iostream>

Server::Server(unsigned port)
    : op::TCPServer(port)
    , mWorkersPool(this)
{
    std::cerr << "open port " << port << std::endl;
}

Server::~Server()
{
    std::cerr << "closing server" << std::endl;
}

void Server::incomingConnection(SOCKET sd, sockaddr * sa, socklen_t sa_len) {
    mWorkersPool.QueueJob(sd);
}

void Server::appendData(std::string data) {
    mWriterPool.QueueJob(std::move(data));
}

