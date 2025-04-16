#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

struct Message {
    std::string nickname;
    std::string text;
    std::string timestamp;
};

std::vector<Message> messages;

std::string getCurrentTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
    return std::string(buf);
}

void sendMessages(int clientSocket) {
    for (const auto& msg : messages) {
        std::string fullMsg = "[" + msg.timestamp + "] " + msg.nickname + ": " + msg.text + "\n";
        send(clientSocket, fullMsg.c_str(), fullMsg.size(), 0);
    }
    std::string end = "=== Історія завершена ===\n";
    send(clientSocket, end.c_str(), end.size(), 0);
}

void handleClient(int clientSocket) {
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) return;
    buffer[bytesReceived] = '\0';
    std::string nickname(buffer);

    std::cout << "[+] " << nickname << " підключився." << std::endl;

    sendMessages(clientSocket);

    while (true) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) break;
        buffer[bytesReceived] = '\0';
        std::string text(buffer);
        std::string timestamp = getCurrentTime();

        Message msg = { nickname, text, timestamp };
        messages.push_back(msg);

        std::cout << "[" << timestamp << "] " << nickname << ": " << text << std::endl;
    }

    close(clientSocket);
    std::cout << "[-] " << nickname << " відключився." << std::endl;
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5555);

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "[*] Сервер очікує на порті 5555..." << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
    return 0;
}
