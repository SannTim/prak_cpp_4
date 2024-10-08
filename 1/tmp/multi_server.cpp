#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>

#define PORT 1337
#define BUFFER_SIZE 1024
#define TIMEOUT_SECONDS 5

int main() {
    // Создание серверного сокета
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation error\n";
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязываем сокет к адресу
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind error\n";
        close(server_fd);
        return 1;
    }

    // Начало прослушивания входящих соединений
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen error\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Server is listening on port " << PORT << "...\n";

    // Набор клиентов
    std::vector<int> client_sockets;
    char buffer[BUFFER_SIZE];

    // Основной цикл работы сервера
    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);

        // Добавляем серверный сокет в набор для select
        FD_SET(server_fd, &read_fds);
        int max_sd = server_fd;

        // Добавляем все клиентские сокеты в набор для select
        for (int client_fd : client_sockets) {
            FD_SET(client_fd, &read_fds);
            if (client_fd > max_sd)
                max_sd = client_fd;
        }

        // Настраиваем тайм-аут на 5 секунд
        timeval timeout;
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;

        // Ожидание активности на одном из сокетов
        int activity = select(max_sd + 1, &read_fds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            std::cerr << "Select error\n";
            break;
        }

        // Если есть входящее соединение на серверном сокете
        if (FD_ISSET(server_fd, &read_fds)) {
            int new_socket = accept(server_fd, nullptr, nullptr);
            if (new_socket < 0) {
                std::cerr << "Accept error\n";
            } else {
                std::cout << "New client connected: " << new_socket << "\n";
                client_sockets.push_back(new_socket); // Добавляем нового клиента в список
            }
        }

        // Обрабатываем данные от каждого клиента
        for (auto it = client_sockets.begin(); it != client_sockets.end(); ) {
            int client_fd = *it;

            if (FD_ISSET(client_fd, &read_fds)) {
                // Чтение данных от клиента
                ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    // Если клиент отключился, закрываем сокет и удаляем из списка
                    std::cout << "Client disconnected: " << client_fd << "\n";
                    close(client_fd);
                    it = client_sockets.erase(it);
                    continue;
                }

                // Эхо: отправляем клиенту обратно то, что получили
                send(client_fd, buffer, bytes_read, 0);
            }

            ++it;
        }

        // Если за 5 секунд не было активности, выводим "blyat"
        if (activity == 0) {
            std::cout << "blyat\n";
        }
    }

    // Закрытие серверного сокета
    close(server_fd);

    return 0;
}

