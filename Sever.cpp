#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <queue>
#include <pthread.h>

using namespace std;

void readBuf(void* args)
{
    int* arg = (int*)args;
    int fd = arg[0];
    int epollfd = arg[1];
    char recvBuf[32] = { 0 };

    int ret = recv(fd, recvBuf, 32, 0);
    if (ret > 0) {
        printf("recv data from client, data: %s\n", recvBuf);

        const char* message = "Hello, client!";
        ret = send(fd, message, strlen(message), 0);

        if (ret == 0) {
            epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
        }

        printf("send to client, data: %s\n", recvBuf);

        if (ret != strlen(recvBuf)) {
            printf("send data error.");
        }
    }
    else {
        printf("recv data error.");
    }
}

class ThreadPool
{
public:
    ThreadPool(int num)
    {
        mutex = PTHREAD_MUTEX_INITIALIZER;
        condition = PTHREAD_COND_INITIALIZER;

        pthread_mutex_init(&mutex, NULL);

        for (int i = 0; i < num; i++)
        {
            pthread_t pid;
            int err = pthread_create(&pid, NULL, ThreadPool::work, this);
            if (err != 0)
            {
                printf("创建工作线程时出现问题");
                return;
            }
        }
    }

    void enqueue(void (*function)(void *),void * arg)
    {
        pthread_mutex_lock(&mutex);
        tasks.push(function);
        taskArgs.push(arg);
        pthread_mutex_unlock(&mutex);

        pthread_cond_signal(&condition);
    }

    ~ThreadPool() {
        for (int i = 0; i < workers.size(); ++i) {
            pthread_join(workers[i], NULL);
        }
    }

private:
    vector<pthread_t> workers;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    queue<void (*)(void *)> tasks;
    queue<void *> taskArgs;

    static void* work(void* arg)
    {
        ThreadPool* pool = (ThreadPool*)arg;
        while (true)
        {
            pthread_mutex_lock(&(pool->mutex));
            while (pool->tasks.empty())
            {
                pthread_cond_wait(&(pool->condition), &(pool->mutex));
            }
            void (*task)(void *) = pool->tasks.front();
            void* arg = pool->taskArgs.front();
            pool->tasks.pop();
            pool->taskArgs.pop();
            pthread_mutex_unlock(&(pool->mutex));

            task(arg);
        }
    }
};

class EpollPool
{
private:
    int epollfd;
    vector<epoll_event> events;
    int serverSocket;
    struct epoll_event ev;
    struct sockaddr_in bindaddr;
    ThreadPool* pool;

public:
    EpollPool(int num) : events(num)
    {
        pool = new ThreadPool(4);
    }

    int creatEpoll(int num)
    {
        serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (serverSocket < 0)
        {
            cout << "无法创建epollfd" << endl;
            return -1;
        }

        bindaddr.sin_family = AF_INET;
        bindaddr.sin_addr.s_addr = INADDR_ANY;
        bindaddr.sin_port = htons(8080);

        if (-1 == bind(serverSocket, (struct sockaddr*)&bindaddr, sizeof(bindaddr))) {
            printf("bind error");
            return -1;
        }

        if (-1 == listen(serverSocket, num))
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

        ev.data.fd = serverSocket;
        ev.events = EPOLLIN | EPOLLET;
        return epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSocket, &ev);
    }

    void work()
    {
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
                }
                else
                {
                    int args[] = { events[i].data.fd, epollfd };
                    pool->enqueue(readBuf, args);
                }
            }
        }

        close(serverSocket);
        close(epollfd);
    }
};

int main() {

    EpollPool pool(10);
    pool.creatEpoll(5);

    pool.work();
    //ThreadPool* pool = new ThreadPool(4);
    return 0;
}