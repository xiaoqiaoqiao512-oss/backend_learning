#include "Server.h"
#include "http/Http.h"
#include "http/Request.h"
#include "http/Response.h"
#include "controller/UserController.h"
#include "middleware/LoggerMiddleware.h"
#include "middleware/TimerMiddleware.h"

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

Server::Server(int port)
    : port_(port),
      server_fd_(-1),
      userController_(userService_)
{
    router_.get(
        "/user/:id",
        std::bind(
            &UserController::getUserById,
            &userController_,
            std::placeholders::_1));
    router_.post(
        "/user",
        std::bind(
            &UserController::createUser,
            &userController_,
            std::placeholders::_1));
}

void Server::start()
{
    createSocket();
    bindSocket();
    listenSocket();
    acceptLoop();
}

void Server::createSocket()
{
    server_fd_ = socket(
        AF_INET,
        SOCK_STREAM,
        0);

    if (server_fd_ == -1)
    {
        throw std::runtime_error(
            "socket failed");
    }
}

void Server::bindSocket()
{
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(
            server_fd_,
            (sockaddr *)&address,
            sizeof(address)) == -1)
    {
        throw std::runtime_error(
            "bind failed");
    }
}

void Server::listenSocket()
{
    if (listen(server_fd_, SOMAXCONN) == -1)
    {
        throw std::runtime_error(
            "listen failed");
    }
}

std::string Server::readRequest(
    int client_fd)
{
    std::string request;

    char buffer[4096];

    while (true)
    {
        int n = recv(
            client_fd,
            buffer,
            sizeof(buffer),
            0);

        if (n <= 0)
        {
            break;
        }

        request.append(
            buffer,
            n);

        if (request.find("\r\n\r\n") != std::string::npos)
        {
            break;
        }
    }

    size_t headerEnd =
        request.find("\r\n\r\n");

    if (headerEnd == std::string::npos)
    {
        return request;
    }

    std::string header =
        request.substr(
            0,
            headerEnd);

    size_t contentLength = 0;

    auto pos =
        header.find(
            "Content-Length");

    if (pos != std::string::npos)
    {
        pos += strlen(
            "Content-Length:");

        while (
            header[pos] == ' ')
        {
            pos++;
        }

        contentLength =
            std::stoi(
                header.substr(pos));
    }

    size_t bodyStart = headerEnd + 4;

    while (
        request.size() - bodyStart <
        contentLength)
    {
        int n = recv(
            client_fd,
            buffer,
            sizeof(buffer),
            0);

        if (n <= 0)
        {
            break;
        }

        request.append(
            buffer,
            n);
    }
    return request;
}

bool sendAll(
    int fd,
    const std::string &data)
{
    size_t total = 0;
    while (total < data.size())
    {
        int sent = send(
            fd,
            data.c_str() + total,
            data.size() - total,
            0);

        if (sent <= 0)
        {
            return false;
        }

        total += sent;
    }

    return true;
}

void Server::acceptLoop()
{
    while (true)
    {
        int client_fd = accept(
            server_fd_,
            nullptr,
            nullptr);

        if (client_fd == -1)
        {
            continue;
        }

        std::string data =
            readRequest(client_fd);

        if (data.empty())
        {
            close(client_fd);
            continue;
        }

        try
        {
            Request req =
                Http::parseRequest(data);

            auto routerNext = 
                [this](Request& request)
                {
                    return router_.handle(request);
                };

            auto timerNext =
                [this, routerNext](Request& request)
                {
                    return timerMiddleware_.handle(
                        request,
                        routerNext
                    );
                };
            
            Response response =
                loggerMiddleware_.handle(
                    req,
                    timerNext
                );

            std::string result = response.serialize();

            sendAll(
                client_fd,
                result);
        }
        catch (const std::exception &e)
        {
            Response response =
                Response::error(
                    400,
                    e.what());
            std::string result =
                response.serialize();
            sendAll(
                client_fd,
                result);
        }

        close(client_fd);
    }
}