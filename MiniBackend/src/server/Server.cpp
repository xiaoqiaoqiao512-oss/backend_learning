#include "Server.h"
#include "http/Http.h"
#include "http/Request.h"
#include "http/Response.h"

#include<iostream>

#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

Server::Server(int port)
    :
    port_(port),
    server_fd_(-1)
{
    router_.get(
        "/hello",
        [](const Request& request)
        {
            Response res;
            res.statusCode = 200;
            res.statusText = "OK";
            res.headers["Content-Type"] = "text/plain";
            res.body = "Hello Router";

            return res;
        }
    );
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
        0
    );

    if(server_fd_ == -1)
    {
        throw std::runtime_error(
            "socket failed"
        );
    }
}

void Server::bindSocket()
{
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    address.sin_addr.s_addr = INADDR_ANY;

    if(bind(
        server_fd_,
        (sockaddr*)&address,
        sizeof(address)
    )==-1)
    {
        throw std::runtime_error(
            "bind failed"
        );
    }
}

void Server::listenSocket()
{
    if(listen(server_fd_, SOMAXCONN)==-1)
    {
        throw std::runtime_error(
            "listen failed"
        );
    }
}

void Server::acceptLoop()
{
    while (true)
    {
        int client_fd = accept(
            server_fd_,
            nullptr,
            nullptr
        );

        char buffer[4096]{};

        int n = recv(
            client_fd,
            buffer,
            sizeof(buffer)-1,
            0
        );   

        if(n > 0){
            std::string data(
                buffer,
                n
            );

            Request req = 
                Http::parseRequest(data);

            Response response = router_.handle(req);

            std::string result = response.serialize();

            send(
                client_fd,
                result.c_str(),
                result.size(),
                0
            );
        }

        close(client_fd);
    }
}