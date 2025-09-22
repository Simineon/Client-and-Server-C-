#include "TCPServer.h"
#include <iostream>

using namespace std;

string TCPServer::Message;
pthread_mutex_t TCPServer::mutex = PTHREAD_MUTEX_INITIALIZER;

void* TCPServer::Task(void *arg) {
    int n;
    int newsockfd = (long)arg;
    char msg[MAXPACKETSIZE];
    pthread_detach(pthread_self());
    
    while(1) {
        n = recv(newsockfd, msg, MAXPACKETSIZE-1, 0);
        if(n <= 0) {
            close(newsockfd);
            break;
        }
        msg[n] = 0;
        
        pthread_mutex_lock(&mutex);
        Message = string(msg);
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

bool TCPServer::setup(int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }
    
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return false;
    }
    
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
    
    // ИСПРАВЛЕНИЕ: правильный вызов bind()
    if (::bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("bind");
        close(sockfd);
        return false;
    }
    
    // ИСПРАВЛЕНИЕ: правильный вызов listen()
    if (::listen(sockfd, 5) < 0) {
        perror("listen");
        close(sockfd);
        return false;
    }
    
    cout << "Server listening on port " << port << endl;
    return true;
}

void TCPServer::receive() {
    while(1) {
        socklen_t sosize = sizeof(clientAddress);
        newsockfd = accept(sockfd, (struct sockaddr*)&clientAddress, &sosize);
        if (newsockfd < 0) {
            perror("accept");
            continue;
        }
        
        cout << "Client connected: " << inet_ntoa(clientAddress.sin_addr) << endl;
        pthread_create(&serverThread, NULL, &Task, (void*)(long)newsockfd);
    }
}

string TCPServer::getMessage() {
    pthread_mutex_lock(&mutex);
    string msg = Message;
    pthread_mutex_unlock(&mutex);
    return msg;
}

bool TCPServer::Send(string msg) {
    if (newsockfd > 0) {
        int result = send(newsockfd, msg.c_str(), msg.length(), 0);
        return result >= 0;
    }
    return false;
}

void TCPServer::clean() {
    pthread_mutex_lock(&mutex);
    Message = "";
    pthread_mutex_unlock(&mutex);
}

void TCPServer::detach() {
    if (newsockfd > 0) {
        close(newsockfd);
        newsockfd = -1;
    }
    if (sockfd > 0) {
        close(sockfd);
        sockfd = -1;
    }
}