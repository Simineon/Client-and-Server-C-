#include "TCPClient.h"

TCPClient::TCPClient() {
    sock = -1;
    port = 0;
    address = "";
}

bool TCPClient::setup(string address, int port) {
    if (sock == -1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            cout << "Could not create socket" << endl;
        }
    }

    if (inet_addr(address.c_str()) == -1) {
        struct hostent *he;
        struct in_addr **addr_list;
        if ((he = gethostbyname(address.c_str())) == NULL) { // Исправлено на gethostbyname
            herror("gethostbyname");
            cout << "Failed to resolve hostname\n";
            return false;
        }
        addr_list = (struct in_addr **) he->h_addr_list;
        for (int i = 0; addr_list[i] != NULL; i++) { // Добавлена инициализация i
            server.sin_addr = *addr_list[i];
            break;
        }
    } else {
        server.sin_addr.s_addr = inet_addr(address.c_str());
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return false;
    }
    return true;
}

bool TCPClient::Send(string data) {
    if (sock != -1) {
        if (send(sock, data.c_str(), data.length(), 0) < 0) { // Исправлено на data.length()
            cout << "Send failed: " << data << endl;
            return false;
        }
    } else 
        return false;
    return true; // Исправлено возвращаемое значение
}

string TCPClient::receive(int size) {
    char *buffer = new char[size]; // Заменен VLA на динамический массив
    memset(buffer, 0, size);

    string reply;
    int bytes_received = recv(sock, buffer, size, 0);
    if (bytes_received < 0) {
        cout << "receive failed!" << endl;
        delete[] buffer;
        return "";
    }
    buffer[bytes_received] = '\0'; // Исправлено присваивание нулевого символа
    reply = buffer;
    delete[] buffer;
    return reply;
}

string TCPClient::read() {
    char buffer[1] = {};
    string reply;
    while (buffer[0] != '\n') { // Исправлено сравнение с символом, а не строкой
        if (recv(sock, buffer, sizeof(buffer), 0) < 0) {
            cout << "receive failed!" << endl;
            return "";
        }
        reply += buffer[0];
    }
    return reply;
}

void TCPClient::exit() {
    close(sock);
}