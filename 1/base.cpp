#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <unistd.h>
#include <coroutine>
#include <deque>
#include <chrono>
#include <sys/socket.h>
#include <vector>
#include <cstdlib>
#include <iostream>
#include "smartpointr.h"
// #include <thread>  // Для std::this_thread::sleep_for
#define BUFFER_SIZE 1024
#define MIN_TO_ACT 3

std::string vectorToString(const std::vector<int>& vec) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i != vec.size() - 1)
            oss << ", ";         
    }
    return oss.str();
}

int conv_to_int(std::string s){
	try {
		return std::stoi(s);
	} catch (std::invalid_argument) {
		return -1;
	}
}

bool valueExists(const std::vector<int>& vec, int value) {
    return std::ranges::find(vec, value) != vec.end();
}



class Client {
public:
	static size_t number_of_clients;
	int client_socket;
	int id;
	bool ready = false;
	bool active = false;
	Client(int sock){
		client_socket = sock;
		number_of_clients ++;
		id = number_of_clients;
		active = true;
	}
	void send_message(const std::string &message) {
        ssize_t bytes_sent = send(client_socket, message.c_str(), message.size(), 0);
        if (bytes_sent < 0) {
            perror("Ошибка при отправке сообщения");
			active = false;
			close(client_socket);
        }
    }
};

struct actions{
	std::vector<int> mafia_votes;
	int will_be_healed = -1;
	int will_be_blessed = -1;
	int will_be_blocked = -1;
	int will_be_checked = -1;
	int will_be_extrakilled = -1;
	int will_be_killed = -1;
	int will_be_killed_byman = -1;
	int don_choice;
};

struct Task {
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;
	
	handle_type coro_handle;
	
	Task(handle_type h) : coro_handle(h) {}
	~Task() {
	    if (coro_handle) coro_handle.destroy();
	}
	
	Task(Task&& other) noexcept : coro_handle(other.coro_handle) {
	    other.coro_handle = nullptr;
	}
	
	Task& operator=(Task&& other) noexcept {
	    if (this != &other) {
	        if (coro_handle) coro_handle.destroy();
	        coro_handle = other.coro_handle;
	        other.coro_handle = nullptr;
	    }
	    return *this;
	}
	
	bool done() const {
	    return coro_handle.done();
	}
	
	void resume() {
	    if (coro_handle && !coro_handle.done()) {
	        coro_handle.resume();
	    }
	}
	
	struct promise_type {
	    auto get_return_object() {
	        return Task{handle_type::from_promise(*this)};
	    }
	    std::suspend_always initial_suspend() { return {}; }  // Приостанавливаем выполнение сразу
	    std::suspend_always final_suspend() noexcept { return {}; }  // Завершаем корректно
	    void return_void() {}
	    void unhandled_exception() { std::terminate(); }
	};
};

class Player{
	public:
		// is player dead or alive
		int killed = -1;
		SmartPointer<Client> ich;
		bool alive = true;
		int sock;
		int against_who = 0;
		char buffer[BUFFER_SIZE];
		static SmartPointer<actions> action;
			
		Player():sock(-1){}
		Player(int sockt):sock(sockt){}
		~Player(){}
		virtual void mafia_private(std::string& message){}
		Task vote(SmartPointer<std::vector<int>> ids, SmartPointer<std::vector<int>> &socks){
			// used to vote for someone at day
			char * endp;
			while (true){
				// std::cout << "Trying to read " << ich->id << "\n";
				ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
				buffer[bytes_read] = '\0';
				co_await std::suspend_always();
				if (bytes_read	>= 0){
					std::string message = std::string(buffer);
					if (message.starts_with("/v ")){
						int temp_id = conv_to_int(message.substr(3));
						//std::cout << temp_id << "\n";
						//std::cout << "h:" << vectorToString(*ids.get()) 
						//<< "   " << valueExists(*ids.get(), against_who) << "\n";
						if (valueExists(*ids.get(), temp_id)){
							against_who = temp_id;
							message = " [Announcement] "+ std::to_string(ich->id) +
								" voted against " + std::to_string(against_who);
							for (auto el: *socks.get()){
								ssize_t bytes_sent = send(el, message.c_str(), message.size(), 0);
							}

							break;
						}
						continue;
					}
					if (message.starts_with("/list")){
						ich->send_message(vectorToString(*ids));
						continue;
					}
					if (message.starts_with("/p ")) {
						mafia_private(message);
						continue;
					}
					message = " [Global] "+ std::to_string(ich->id) + ": " + message;
					for (auto el: *socks.get()){
						ssize_t bytes_sent = send(el, message.c_str(), message.size(), 0);
					}
				}
				co_await std::suspend_always();
			}
			co_return;
		}
		virtual Task act(SmartPointer<std::vector<int>> ids){co_return;}


};

class Mafia: public Player{
public:
	static std::vector<SmartPointer<Mafia>> mafiosi;
	bool don = false;
	void mafia_private(std::string& message){
		std::string nmes = " [Mafia private] " + std::to_string(ich->id) + ": " + message.substr(3);
		for (auto cl: mafiosi)
			cl->ich->send_message(nmes);
	}	
	virtual void kill(int pl){
		action->mafia_votes.push_back(pl);
		killed = pl;
		if (don) action->don_choice = pl;
	}
	Task act(SmartPointer<std::vector<int>> ids){
		while(true){
			ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
			buffer[bytes_read] = '\0';
			co_await std::suspend_always();
			if (bytes_read	>= 0){
				std::string message = std::string(buffer);
				if (message.starts_with("/p ")) {
					mafia_private(message);
					continue;
				}
				if (message.starts_with("/k ")){
					int temp_id = conv_to_int(message.substr(3));
					if (valueExists(*ids.get(), temp_id)){
						kill(temp_id);
						ich->send_message("You voted to kill " + std::to_string(temp_id) + "\n");
						message = "    Voted to kill " + std::to_string(temp_id);
						mafia_private(message);
						break;
					}
					continue;

				}
			}

		}
	}
	Mafia(int sock = -1) : Player(sock) {
		mafiosi.push_back(SmartPointer<Mafia>(this));
	}
};

std::vector<SmartPointer<Mafia>> Mafia::mafiosi;
SmartPointer<actions> Player::action(new actions);

class Doc: public Player{
	public:
	int prev_heal = -1;
	Doc(int sock = -1) : Player(sock){}
	Task act(SmartPointer<std::vector<int>> ids){
		while(true){
			ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
			buffer[bytes_read] = '\0';
			co_await std::suspend_always();
			
			if (bytes_read	>= 0){
				std::string message = std::string(buffer);
				if (message.starts_with("/h ")){
					int temp_id = conv_to_int(message.substr(3));
					if (temp_id != prev_heal && valueExists(*ids.get(), temp_id)){
						action->will_be_healed = temp_id;
						ich->send_message("You healed " + std::to_string(temp_id) + "\n");
						prev_heal = temp_id;
						break;
					}
					continue;

				}
			}

		}
	}
};

class Kom: public Player{
	public:
	int killed = -1;
	Kom(int sock = -1) : Player(sock){}
	Task act(SmartPointer<std::vector<int>> ids){
		while(true){
			ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
			buffer[bytes_read] = '\0';
			co_await std::suspend_always();
			if (bytes_read	>= 0){
				std::string message = std::string(buffer);
				if (message.starts_with("/c ")){
					int temp_id = conv_to_int(message.substr(3));
					if (valueExists(*ids.get(), temp_id)){
						action->will_be_checked = temp_id;
						ich->send_message("You check " + std::to_string(temp_id) + "\n");

						break;
					}
					continue;
				}
				if (message.starts_with("/k ")){
					int temp_id = conv_to_int(message.substr(3));
					if (valueExists(*ids.get(), temp_id)){
						action->will_be_killed = temp_id;
						ich->send_message("You kill " + std::to_string(temp_id) + "\n");

						killed = temp_id;
						break;
					}
					continue;
				}
			}
		}
	}	
};

class Man: public Player{
	public:
	Man(int sock = -1) : Player(sock){}
	Task act(SmartPointer<std::vector<int>> ids){
		while(true){
			ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
			buffer[bytes_read] = '\0';
			co_await std::suspend_always();
			if (bytes_read	>= 0){
				std::string message = std::string(buffer);
				if (message.starts_with("/k ")){
					int temp_id = conv_to_int(message.substr(3));
					if (valueExists(*ids.get(), temp_id)){
						action->will_be_killed_byman = temp_id;
						killed = temp_id;
						ich->send_message("You killed " + std::to_string(temp_id) + "\n");

						break;
					}
					continue;
				}
			}
		}
	}	

};

class Kil: public Mafia{
	public:
	Kil(int sock = -1) : Mafia(sock){} 
	void kill (int pl){
		killed = pl;
		action->will_be_extrakilled = pl;
	}
};

class Priest: public Player{
	public:
	Priest(int sock = -1) : Player(sock){}

	Task act(SmartPointer<std::vector<int>> ids){
		while(true){
			ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
			buffer[bytes_read] = '\0';
			co_await std::suspend_always();
			if (bytes_read	>= 0){
				std::string message = std::string(buffer);
				if (message.starts_with("/b ")){
					int temp_id = conv_to_int(message.substr(3));
					if (valueExists(*ids.get(), temp_id)){
						action->will_be_blessed = temp_id;
						ich->send_message("You bless " + std::to_string(temp_id) + "\n");

						break;
					}
					continue;
				}
			}
		}
	}	

};

class Samurai: public Player{
	public:
	Samurai(int sock = -1) : Player(sock){}
	Task act(SmartPointer<std::vector<int>> ids){
		while(true){
			ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
			buffer[bytes_read] = '\0';
			co_await std::suspend_always();
			if (bytes_read	>= 0){
				std::string message = std::string(buffer);
				if (message.starts_with("/b ")){
					int temp_id = conv_to_int(message.substr(3));
					if (valueExists(*ids.get(), temp_id)){
						action->will_be_blocked = temp_id;
						ich->send_message("You block " + std::to_string(temp_id) + "\n");

						break;
					}
					continue;
				}
			}
		}
	}	

};

template <typename T>
class has_act_method {
private:
    // Проверка на наличие метода act
    template <typename U>
    static auto test(int) -> decltype(std::declval<U>().act(), std::true_type());

    template <typename U>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
bool checkActMethod(T& obj) {
    if constexpr (has_act_method<T>::value) {
        return true;
    } else {
		return false;
    }
}

void event_loop(std::deque<Task>& tasks) {
    auto start_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::minutes(MIN_TO_ACT);

    while (!tasks.empty()) {
        for (auto it = tasks.begin(); it != tasks.end();) {
            it->resume();  // Резюмируем корутину
            if (it->done()) {
                it = tasks.erase(it);  // Убираем задачу, если она завершена
				// std::cout << "Remaining tasks: " << tasks.size();
				
            } else {
                ++it;  // Переходим к следующей задаче
            }

            // Проверка времени
            auto current_time = std::chrono::steady_clock::now();
            if (current_time - start_time >= duration) {
                return;  // Завершаем выполнение функции
            }
        }
    }
	// std::this_thread::sleep_for(std::chrono::milliseconds(10000));
}
