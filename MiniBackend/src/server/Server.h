#pragma once

#include "router/Router.h"

class Server
{
public:
    Server(int port);
    void start();

private:
    void createSocket();
    void bindSocket();
    void listenSocket();
    void acceptLoop();
    std::string readRequest(
        int client_fd
    );

private:
    int port_;
    int server_fd_;
    Router router_;
};