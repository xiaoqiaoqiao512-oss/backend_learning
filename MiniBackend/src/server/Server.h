#pragma once

#include "router/Router.h"
#include "service/UserService.h"
#include "controller/UserController.h"
#include "middleware/LoggerMiddleware.h"
#include "middleware/TimerMiddleware.h"

class Server
{
public:
    Server(int port);
    void start();

private:
    void createSocket();
    void configureSocket();
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

    UserService userService_;
    UserController userController_;
    LoggerMiddleware loggerMiddleware_;
    TimerMiddleware timerMiddleware_;
};