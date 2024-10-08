#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1337
#define BUFFER_SIZE 1024

int main() {
    // Создание сокета
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return 1;
    }

    // Определение адреса сервера
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // Преобразуем IP-адрес из строки
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address or address not supported\n";
        close(sock);
        return 1;
    }

    // Подключение к серверу
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        close(sock);
        return 1;
    }

    std::cout << "Connected to the server at " << SERVER_IP << ":" << SERVER_PORT << "\n";

    // Цикл отправки и получения данных
    char buffer[BUFFER_SIZE];
    std::string message;

    while (true) {
        std::cout << "Enter message (or 'exit' to quit): ";
        std::getline(std::cin, message);

        // Если пользователь вводит 'exit', выходим из программы
        if (message == "exit") {
            break;
        }

        // Отправляем сообщение серверу
        send(sock, message.c_str(), message.length(), 0);

        // Чтение ответа от сервера
        ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            std::cerr << "Connection closed by server or read error\n";
            break;
        }

        // Вывод ответа сервера
        std::cout << "Server echoed: " << std::string(buffer, bytes_received) << "\n";
    }

    // Закрываем сокет
    close(sock);
    return 0;
}

