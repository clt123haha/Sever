#include <queue>
#include <vector>
#include "src/EventLoop.h"
#include "src/Server.h"


int main() {
    EventLoop* loop = new EventLoop();
    Server* sever = new Server(loop);
    loop->loop();
    return 0;
}

