#include "Connection.h"
#include <string.h>

Connection::Connection(EventLoop* _loop, Socket* _sock) : loop(_loop), sock(_sock), channel(nullptr), readBuffer(nullptr) {
    channel = new Channel(loop, sock->getFd());
    channel->enableRead();
    channel->useET();
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setReadCallback(cb);
    readBuffer = new Buffer();
}

Connection::~Connection() {
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> _cb) {
    deleteConnectionCallback = _cb;
}


void Connection::echo(int sockfd) {
    char buf[1024];     //���buf��С����ν
    while (true) {    //����ʹ�÷�����IO����ȡ�ͻ���buffer��һ�ζ�ȡbuf��С���ݣ�ֱ��ȫ����ȡ���
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            readBuffer->append(buf, bytes_read);
        }
        else if (bytes_read == -1 && errno == EINTR) {  //�ͻ��������жϡ�������ȡ
            printf("continue reading\n");
            continue;
        }
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {//������IO�����������ʾ����ȫ����ȡ���
            printf("message from client fd %d: %s\n", sockfd, readBuffer->c_str());
            // errif(write(sockfd, readBuffer->c_str(), readBuffer->size()) == -1, "socket write error");
            send(sockfd);
            readBuffer->clear();
            break;
        }
        else if (bytes_read == 0) {  //EOF���ͻ��˶Ͽ�����
            printf("EOF, client fd %d disconnected\n", sockfd);
            deleteConnectionCallback(sockfd);
            break;
        }
        else {
            printf("Connection reset by peer\n");
            deleteConnectionCallback(sockfd);
            break;
        }
    }
}

void Connection::send(int sockfd) {
    char buf[readBuffer->size()];
    strcpy(buf, readBuffer->c_str());
    int  data_size = readBuffer->size();
    int data_left = data_size;
    while (data_left > 0)
    {
        ssize_t bytes_write = write(sockfd, buf + data_size - data_left, data_left);
        if (bytes_write == -1 && errno == EAGAIN) {
            break;
        }
        data_left -= bytes_write;
    }
}