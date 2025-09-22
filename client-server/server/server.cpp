#include <iostream>
#include <signal.h>
#include "TCPServer.h"

TCPServer tcp;
bool running = true;

void sig_exit(int s) {
    running = false;
    tcp.detach();
    exit(0);
}

void * loop(void * m) {
    pthread_detach(pthread_self());
    while(running) {
        string str = tcp.getMessage();
        if(str != "") {
            cout << "Message:" << str << endl;
            tcp.Send(" [server received: "+str+"] ");
            tcp.clean();
        }
        usleep(100000); // 100ms вместо 1ms
    }
    return NULL;
}

int main() {
    signal(SIGINT, sig_exit);
    
    pthread_t msg;
    if (tcp.setup(11999)) {
        if (pthread_create(&msg, NULL, loop, NULL) == 0) {
            tcp.receive(); // Этот вызов будет блокирующим
        }
    }
    return 0;
}