#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <termios.h>

std::string current_input;
bool not_end = true;
void setNonCanonicalMode(bool enable) {
    struct termios settings;
    tcgetattr(STDIN_FILENO, &settings);
    if (enable) {
        settings.c_lflag &= ~(ICANON | ECHO);  // Отключаем канонический режим и вывод символов
    } else {
        settings.c_lflag |= ICANON | ECHO;     // Включаем обратно
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}

class Client {
public:
    Client(const std::string& host, int port) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }
    }

    void send_message(const std::string& message) {
        send(sock, message.c_str(), message.size(), 0);
    }

    void read_message() {
        int line_count = 0;  // Количество сообщений

        while (not_end) {
            char temp_buffer[1024];  // Временный буфер для чтения сообщений
            int valread = read(sock, temp_buffer, sizeof(temp_buffer) - 1);
            if (valread > 0) {
                temp_buffer[valread] = '\0';  // Завершаем строку
				std::cout << "\r" << temp_buffer << "\n" << "> " << current_input << std::flush;
								

            } else if (valread == 0) {
                std::cout << "Server closed the connection." << std::endl;
				not_end = false;
                break;
            } else {
                perror("Read error");
				not_end = false;
                break;
            }
        }
    }

    ~Client() {
        close(sock);
    }

private:
    int sock;
};

void send_loop(Client& client) {
    std::string message;
	setNonCanonicalMode(true);
	char ch;
    while (not_end) {
        std::cout << "\n> ";  // Prompt для ввода сообщения
		current_input = "";
		while (not_end) {
			ch = std::cin.get(); // Считываем символы без ожидания Enter

			if (ch == '\n') {    // Если введена клавиша Enter - выходим
				current_input += "\0";
				break;
			}

			current_input += ch; // Добавляем символ в строку

			// Очищаем текущую строку и выводим её заново
			std::cout << "\r> " << current_input << std::flush;
		}
		if (not_end)
			client.send_message(current_input);
    }
	setNonCanonicalMode(false);
}

int main() {
    Client client("127.0.0.1", 1337);
    std::thread read_thread(&Client::read_message, &client);
    send_loop(client);
    read_thread.join();
    return 0;
}

