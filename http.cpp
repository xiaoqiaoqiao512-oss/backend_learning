#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    // 1. 创建 socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        std::cerr << "socket 创建失败\n";
        return 1;
    }

    // 2. 构造地址
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = INADDR_ANY;

    // 3. 绑定端口
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == -1)
    {
        std::cerr << "bind 失败\n";
        close(server_fd);
        return 1;
    }

    // 4. 开始监听
    if (listen(server_fd, 10) == -1)
    {
        std::cerr << "listen 失败\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Server Start!" << std::endl;

    while (true)
    {
        std::cout << "等待客户端连接..." << std::endl;

        // 5. 接收连接
        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd == -1)
        {
            std::cerr << "accept 失败\n";
            continue;
        }

        std::cout << "客户端连接成功！" << std::endl;

        // 6. 返回数据
        const char* msg =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello Backend";

        char buffer[4096]{};

        int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        std::cout << "收到的数据：" << std::endl;

        std::cout << buffer << std::endl;
        send(client_fd, msg, strlen(msg), 0);

        // 7. 关闭客户端连接
        close(client_fd);

        std::cout << "客户端断开\n\n";
    }

    close(server_fd);

    return 0;
}