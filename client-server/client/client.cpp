#include <iostream>
#include <signal.h>
#include <chrono>
#include <thread>
#include <string>
#include "TCPClient.h"

TCPClient tcp;
bool running = true;

void sig_exit(int s) {
    running = false;
    tcp.exit();
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sig_exit);

    if (!tcp.setup("127.0.0.1", 11999)) {
        cerr << "Failed to connect to server" << endl;
        return 1;
    }

    cout << "Connected to server" << endl;
    cout << "Type your messages (type 'quit' to exit):" << endl;
    
    int counter = 0;
    while(running) {
        string message;
        
        // Запрашиваем сообщение у пользователя
        cout << "Enter message " << (counter + 1) << ": ";
        getline(cin, message);
        
        // Проверяем, не хочет ли пользователь выйти
        if (message == "quit" || message == "exit") {
            cout << "Exiting..." << endl;
            break;
        }
        
        // Проверяем, не пустое ли сообщение
        if (message.empty()) {
            cout << "Message cannot be empty. Try again." << endl;
            continue;
        }
        
        // Отправляем сообщение
        if (tcp.Send(message)) {
            cout << "Sent: " << message << endl;
            counter++;
        } else {
            cerr << "Failed to send message" << endl;
            break;
        }
        
        // Получаем ответ от сервера
        string rec = tcp.receive(1024);
        if (!rec.empty()) {
            cout << "Server Response: " << rec << endl;
        }
        
        // Небольшая пауза перед следующим запросом
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}