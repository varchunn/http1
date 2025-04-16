#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void receiveMessages(int clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) break;
        buffer[bytesReceived] = '\0';
        std::cout << buffer << std::endl;
    }
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Не вдалося створити сокет\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5555);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Помилка підключення до сервера\n";
        return 1;
    }

    std::string nickname;
    std::cout << "Введіть ваше ім’я: ";
    std::getline(std::cin, nickname);

    send(clientSocket, nickname.c_str(), nickname.size(), 0);

    std::thread t(receiveMessages, clientSocket);
    t.detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "/exit") break;
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    close(clientSocket);
    return 0;
}
