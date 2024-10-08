#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 1337
#define BUFFER_SIZE 1024
#define TIMEOUT_SECONDS 5

int main() {
    // Создание сокета
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation error\n";
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Принимаем соединения на всех интерфейсах
    server_addr.sin_port = htons(PORT); // Указываем порт

    // Привязываем сокет к адресу
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind error\n";
        close(server_fd);
        return 1;
    }

    // Начало прослушивания входящих соединений
    if (listen(server_fd, 1) < 0) {
        std::cerr << "Listen error\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Server is listening on port " << PORT << "...\n";

    // Принятие входящего соединения
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        std::cerr << "Accept error\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Client connected!\n";

    // Основной цикл обработки данных
    char buffer[BUFFER_SIZE];
    fd_set read_fds;
    timeval timeout;

    while (true) {
        // Очищаем и добавляем клиентский сокет в набор для select
        FD_ZERO(&read_fds);
        FD_SET(client_fd, &read_fds);

        // Настраиваем тайм-аут на 5 секунд
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;

        // Используем select для ожидания данных или тайм-аута
        int activity = select(client_fd + 1, &read_fds, nullptr, nullptr, &timeout);

        if (activity < 0) {
            std::cerr << "Select error\n";
            break;
        } else if (activity == 0) {
            // Если тайм-аут (за 5 секунд не поступили данные)
            std::cout << "blyat\n";
        } else {
            // Данные пришли, читаем их
            ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                std::cerr << "Client disconnected or read error\n";
                break;
            }

            // Эхо: отправляем клиенту обратно то, что получили
            send(client_fd, buffer, bytes_read, 0);
        }
    }

    // Закрытие соединения и сокета
    close(client_fd);
    close(server_fd);

    return 0;
}

