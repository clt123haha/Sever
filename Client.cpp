#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

using namespace std;

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    const char* message = "Hello, server!";
    send(clientSocket, message, strlen(message), 0);

    char buf[32];
    int s = recv(clientSocket, buf, 32, 0);
    cout << buf << endl;
    return 0;
}

