#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>

using namespace std;

int main() {
    int epollfd;
    vector<epoll_event> events(10);

    int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK,0);
    if (serverSocket < 0)
    {
        cout << "无法创建epollfd" << endl;
        return -1;
    }

    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = INADDR_ANY;
    bindaddr.sin_port = htons(8080);

    if (-1 == bind(serverSocket, (struct sockaddr*)&bindaddr, sizeof(bindaddr))) {
        printf("bind error");
        return -1;
    }

    if(-1 == listen(serverSocket, 5))
    {
        printf("listen error");
        return -1;
    }

    epollfd = epoll_create1(0);
    if (epollfd < 0)
    {
        perror("Epoll Create");
        return -1;
    }

    struct epoll_event ev;
    ev.data.fd = serverSocket;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSocket, &ev);

    while (true)
    {
        int num = epoll_wait(epollfd, events.data(), 10, -1);

        if (num < 0)
        {
            perror("epoll failed");
            break;
        }

        for (int i = 0; i < num; i++)
        {
            if (events[i].events & EPOLLIN && events[i].data.fd == serverSocket)
            {
                struct sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

                ev.events = EPOLLIN | EPOLLET;
                fcntl(clientSocket, F_SETFL, O_NONBLOCK);

                ev.data.fd = clientSocket;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSocket, &ev);
                cout << clientSocket << " add\n" << endl;
            }
            else
            {
                /*char recvBuf[32] = {0};

                int ret = recv(events[i].data.fd, recvBuf, 32, 0);
                if (ret > 0) {
                    printf("recv data from client, data: %s\n", recvBuf);

                    ret = send(events[i].data.fd, recvBuf, strlen(recvBuf), 0);

                    if (ret != strlen(recvBuf)) {
                        printf("send data error.");
                    }
                }
                else {
                    printf("recv data error.");
                }*/
                char buffer[1024];
                int bytesRead = read(events[i].data.fd, buffer, 1024);
                if (bytesRead == 0) {
                    close(events[i].data.fd);
                }
                else {
                    buffer[bytesRead] = '\0';
                    std::cout << "Received message: " << buffer << std::endl;
                }
            }
        }
    }

    close(serverSocket);
    close(epollfd);

    return 0;
}
