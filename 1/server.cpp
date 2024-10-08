#include <cstddef>
#include <iostream>
#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <algorithm>
#include <ranges>
#include <random>
#include "base.cpp"
#include "logger.h"
#include <deque>
#include <unordered_map>
#define PORT 1337
#define BUFFER_SIZE 1024
#define TIMEOUT_SECONDS 20
#define MIN_PALYERS 4
#define DELIMETR 3
#define ALLOW_DOC true
#define ALLOW_KOM true 
#define ALLOW_MAN true
#define ALLOW_PRI true 
#define ALLOW_KILLER true
#define ALLOW_SAMURAI true

Logger logger;
size_t Client::number_of_clients = 0;


struct stasistics{
	int man_kills = 0;
	int mafia_kills = 0;
	int doctor_heals = 0;
	int priest_checks = 0;
	int kom_checks = 0;
	int kom_kills = 0;
	int voted_out = 0;
	bool succesful_samurai = false;

	std::string to_string() const {
        std::ostringstream oss;
        oss << "Statistics:\n"
            << "Man Kills: " << man_kills << "\n"
            << "Mafia Kills: " << mafia_kills << "\n"
            << "Doctor Heals: " << doctor_heals << "\n"
            << "Priest Checks: " << priest_checks << "\n"
            << "Kom Checks: " << kom_checks << "\n"
            << "Kom Kills: " << kom_kills << "\n"
            << "Voted Out: " << voted_out << "\n"
            << "Successful Samurai: " << (succesful_samurai ? "Yes" : "No") << "\n";
        return oss.str();
    }
};
stasistics stat;

void set_non_blocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

int mostFrequentElement(const std::vector<int>& vec, int don) {
    std::unordered_map<int, int> countMap;
    
    // Подсчет вхождений элементов
    for (int num : vec) {
        countMap[num]++;
    }

    int maxCount = 0;
    int mostFrequent = -1;
    int frequencyCount = 0;

    // Поиск самого частого элемента
    for (const auto& pair : countMap) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            mostFrequent = pair.first;
            frequencyCount = 1; // сбрасываем счетчик
        } else if (pair.second == maxCount) {
            frequencyCount++;
        }
    }

    // Проверка на наличие нескольких самых частых элементов
	int choice = frequencyCount > 1 ? don : mostFrequent;

    return choice == -1 ? mostFrequent: choice;
}

class Game{
	std::vector<SmartPointer<Client>> clients;
	std::vector<SmartPointer<Player>> players;
	char buffer[BUFFER_SIZE];
	int server_fd;
    sockaddr_in address;
	int max_sd;
	int day_counter;
	size_t num_of_ready;
	SmartPointer<Player> sam_id, kom_id, pri_id;

public:
	bool not_done = false;
    Game(int port) {
		server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd == -1) {
    	    std::cerr << "Socket creation error\n";
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
    	}

    	// Начало прослушивания входящих соединений
    	if (listen(server_fd, 1) < 0) {
    	    std::cerr << "Listen error\n";
    	    close(server_fd);
    	}
		max_sd = server_fd;
    	std::cout << "Server is listening on port " << PORT << "...\n";


    }
	~Game(){
		close(server_fd);
	}

    void run() {
		std::pair<bool, std::string> result(false, "");
			while (true) {
				std::cout << "New Game has been started\n";
				in_lobby();
				give_roles();
				if (clients.size() < 4) return;
				day_counter = 0;
				while (!result.first){
					night();
					result = check_endgame();
					if (!result.first)
						day();
					result = check_endgame();
				}
				send_all(result.second + " won the game!\n");
				logger.logFinal(result.second + " won the game!");
				logger.logFinal(stat.to_string());
				for (auto pl:players)
					close(pl->ich->client_socket);
				clients.clear();
				players.clear();
				Client::number_of_clients = 0;
				logger = Logger();
			}
	}

private:
	void send_all(const std::string &message){
		for (auto cl: clients)
			cl->send_message(message);

	}
    void give_roles(){
		int num_of_mafiozi = num_of_ready/DELIMETR;
		int mirnie_cur = 0, mafiosi_cur = 0;
		// Создаем генератор случайных чисел
		std::random_device rd;
		std::mt19937 gen(rd());

		// Перемешиваем вектор
		std::shuffle(clients.begin(), clients.end(), gen);
		//ком включен всегда
		auto it = clients.begin();
		for (int i = 0; i < num_of_mafiozi; i++){
			if (i == num_of_mafiozi-1 && num_of_ready >= 8 && ALLOW_KILLER){
				// players.push_back(Player())
				SmartPointer<Kil> cur_pointer(new Kil(it->get()->client_socket));
				cur_pointer->ich = *it;
					it->get()->send_message("You are mafia!\n");
					it->get()->send_message("But not just normal mafia, you are Killer, you are insane dude with extra kill\n");
					it->get()->send_message("Use /p in front of message to talk in Mafia private\n");
					it->get()->send_message("Use /k <id> to kill someone\n");
					logger.logRound(0, std::to_string(it->get()->id )+": killer");
					std::cout << it->get()->id <<": killer\n";
				players.push_back(SmartPointer<Player>(cur_pointer));

			} else {
				// clients.push_back(SmartPointer<Client>(new Client(new_socket)));
				SmartPointer<Mafia> cur_pointer(new Mafia(it->get()->client_socket));
				cur_pointer->ich = *it;
				if(i == 0){
					cur_pointer->don = true;
					it->get()->send_message("You are mafia!\n");
					it->get()->send_message("But not just normal mafia, you are Boss, your vote is most valuable\n");
					it->get()->send_message("Use /p in front of message to talk in Mafia private\n");
					it->get()->send_message("Use /k <id> to vote to kill someone\n");
					std::cout << it->get()->id <<": DON\n";
					logger.logRound(0, std::to_string(it->get()->id )+": Don");

				} else {
					it->get()->send_message("You are mafia!\n");
					it->get()->send_message("Use /p in front of message to talk in Mafia private\n");
					it->get()->send_message("Use /k <id> to vote to kill someone\n");
					std::cout << it->get()->id <<": Mafia\n";
					logger.logRound(0, std::to_string(it->get()->id )+": Mafia");
				}
				players.push_back(SmartPointer<Player>(cur_pointer));
			}
			it++;
		}
		

		if (num_of_ready > 5 && ALLOW_DOC){ // включаем дока
			SmartPointer<Doc> cur_pointer(new Doc(it->get()->client_socket));
			cur_pointer->ich = *it;
			it->get()->send_message("You are Doctor!\n");
			it->get()->send_message("Use /h <id> to heal someone\n");
			std::cout << it->get()->id <<": Doctor\n";
			logger.logRound(0, std::to_string(it->get()->id )+": Doctor");
			players.push_back(SmartPointer<Player>(cur_pointer));
			it++;
		}
		if (num_of_ready > 7 && ALLOW_MAN) { // включаем маньяка
			SmartPointer<Man> cur_pointer(new Man(it->get()->client_socket));
			cur_pointer->ich = *it;	
			it->get()->send_message("You are Maniac!\n");
			it->get()->send_message("Use /k <id> to vote to kill someone\n");
			std::cout << it->get()->id <<": Maniac\n";
			logger.logRound(0, std::to_string(it->get()->id )+": Man");
			players.push_back(SmartPointer<Player>(cur_pointer));
			it++;
		}
		if (num_of_ready >= 8) { // включаем святошу, киллера и самурая
			if(ALLOW_PRI){
				SmartPointer<Priest> cur_pointer(new Priest(it->get()->client_socket));
				cur_pointer->ich = *it;
				it->get()->send_message("You are Priest!\n");
				it->get()->send_message("Use /b <id> to bless someone\n");
				std::cout << it->get()->id <<": Priest\n";
				logger.logRound(0, std::to_string(it->get()->id )+": Priest");
				pri_id = cur_pointer;
				players.push_back(SmartPointer<Player>(cur_pointer));
				it++;
			}
			
			if(ALLOW_SAMURAI){
				SmartPointer<Samurai> cur_pointer(new Samurai(it->get()->client_socket));
				cur_pointer->ich = *it;
				it->get()->send_message("You are Samurai!\n");
				it->get()->send_message("You just have your way!\n");
				it->get()->send_message("Use /b <id> to protect someone\n");
				std::cout << it->get()->id <<": Samurai\n";
				logger.logRound(0, std::to_string(it->get()->id )+": Samurai");
				sam_id = cur_pointer;
				players.push_back(SmartPointer<Player>(cur_pointer));
				it++;
			}
		} 
		if(ALLOW_KOM){
			SmartPointer<Kom> cur_pointer(new Kom(it->get()->client_socket));
			cur_pointer->ich = *it;
			it->get()->send_message("You are Kom!\n");
			it->get()->send_message("Use /c <id> to check someone\n");
			it->get()->send_message("Use /k <id> to vote to kill someone\n");
			std::cout << it->get()->id <<": Kom\n";
			logger.logRound(0, std::to_string(it->get()->id )+": Kom");
			kom_id = cur_pointer;
			players.push_back(SmartPointer<Player>(cur_pointer));
			it++;
		}
		while(it != clients.end()){
			SmartPointer<Player> cur_pointer(new Player(it->get()->client_socket));
			cur_pointer->ich = *it;
			it->get()->send_message("You are innocent!\n");
			std::cout << it->get()->id <<": Innocent\n";
			logger.logRound(0, std::to_string(it->get()->id )+": Innocent");
			players.push_back(SmartPointer<Player>(cur_pointer));

			it++;
		}

		std::sort(players.begin(), players.end(), [](const SmartPointer<Player>& a, const SmartPointer<Player>& b) {
			return a->ich->id < b->ich->id; // Сравниваем id
		});
		send_all("To vote for someone to get eliminated use /v <id>\n");

	}

	void day(){	
		day_counter++; 
		logger.logRound(day_counter, "Day " + std::to_string(day_counter));
		std::cout << "Day " << day_counter <<"\n";
		std::deque<Task> tasks;
		SmartPointer<std::vector<int>> ids(new std::vector<int>());
		SmartPointer<std::vector<int>> socks(new std::vector<int>());
		for (auto pl: players)
			if (pl->alive){
				ids->push_back(pl->ich->id);
				socks->push_back(pl->sock);
			}
		send_all("Alive players: " + vectorToString(*ids) + "\n");
		for (auto pl: players){
			if (pl->alive)
				tasks.push_back(pl->vote(ids, socks));
		}
		logger.logRound(day_counter, "Alive players: " + vectorToString(*ids));
		std::cout <<"Alive players: " << vectorToString(*ids) << "\n";
		event_loop(tasks);
		int max_frequency = 0;
		std::vector<int> most_frequent_ids;
		std::unordered_map<int, int> frequency;
		auto idis = players | std::views::transform([](const SmartPointer<Player>& ptr) { return ptr->against_who; });
		
		std::ranges::for_each(idis, [&frequency](int id) { frequency[id]++; });	
		
		for (const auto& [id, count] : frequency) {
			if (count > max_frequency) {
				max_frequency = count;
				most_frequent_ids = {id}; // Сбрасываем список, если нашли больше
			} else if (count == max_frequency) {
			    most_frequent_ids.push_back(id); // Добавляем id с той же частотой
			}
		}
		if (most_frequent_ids.size() == 1) {
		     // most_frequent_ids[0] // kill em
   	 	    send_all(" [Announcement] " + std::to_string(most_frequent_ids[0]) + " was killed\n");
			std::cout <<std::to_string(most_frequent_ids[0]) + " was killed\n";
			
			for (auto pl: players){
				if (pl->ich->id == most_frequent_ids[0]){
					pl->alive = false;
					break;
				}
			}
			stat.voted_out++;
   	 	} else {
   	 	    send_all(" [Announcement] Nobody was killed\n");
			std::cout << "Nobody was killed\n";
			logger.logRound(day_counter, "Nobody was killed\n");
   	 	}

	}

	void night(){
		std::cout << "Night: " << day_counter <<"\n";
		std::deque<Task> tasks;
		SmartPointer<std::vector<int>> ids(new std::vector<int>());
		for (auto pl: players)
			if (pl->alive){
				ids->push_back(pl->ich->id);
			}
		send_all("Night starts\nAlive players: " + vectorToString(*ids) +"\n");
		logger.logRound(day_counter, "Alive players: " + vectorToString(*ids));
		std::cout <<"Alive players: " << vectorToString(*ids) << "\n";

		SmartPointer<actions> cur_action(new actions);
		Player::action = cur_action;
		for (auto pl: players){
			if (pl->alive)
				tasks.push_back(pl->act(ids));
		}
		event_loop(tasks);

		std::cout << "Processing night results\n";
		std::set<int> killed;
		killed.insert(cur_action->will_be_extrakilled);
		if (cur_action->will_be_extrakilled != -1)
			logger.logRound(day_counter,"Killer tries to kill " + std::to_string(cur_action->will_be_extrakilled));

		killed.insert(cur_action->will_be_killed);
		if (cur_action->will_be_killed != -1)
			logger.logRound(day_counter,"Kom tries to kill " + std::to_string(cur_action->will_be_killed));

		if (cur_action->will_be_killed_byman != -1)
			logger.logRound(day_counter,"Man tries to kill " + std::to_string(cur_action->will_be_killed_byman));
		killed.insert(cur_action->will_be_killed_byman);

		if (mostFrequentElement(cur_action->mafia_votes, cur_action->don_choice) != -1)
			logger.logRound(day_counter,"Mafia tries to kill " +
					std::to_string(mostFrequentElement(cur_action->mafia_votes, cur_action->don_choice)));
		killed.insert(mostFrequentElement(cur_action->mafia_votes, cur_action->don_choice));
		
		killed.erase(-1);
		if (killed.contains(cur_action->will_be_blocked)){
			//добавить самурая и того кто стрелял
			for(auto pl: players){
				if (pl->killed == cur_action->will_be_blocked){
					logger.logRound(day_counter, "Samurai protects "+ 
									std::to_string(cur_action->will_be_blocked) +
									" and kills " + std::to_string(pl->ich->id));

					killed.insert(sam_id->ich->id);
					killed.insert(pl->ich->id);
					killed.erase(cur_action->will_be_blocked);
					stat.succesful_samurai = true;
					break;
				}
			}
		}
		if (killed.contains(cur_action->will_be_healed)){
			logger.logRound(day_counter, "Doc saved "+ 
									std::to_string(cur_action->will_be_healed));
			stat.doctor_heals++;
			killed.erase(cur_action->will_be_healed);
		}
		//добавить логику для ком и святошы
		SmartPointer<Player> will_be_checked_by_kom, will_be_checked_by_pri;
		for (auto pl: players){
			if (pl->ich->id == cur_action->will_be_checked){
				will_be_checked_by_kom = pl;
			}
			if (pl->ich->id == cur_action->will_be_blessed){
				will_be_checked_by_pri = pl;
			}			
		}
		if (cur_action->will_be_checked != -1){
			Player* tmp = will_be_checked_by_kom.get();
			std::string role = std::string(typeid(*tmp).name()).substr(1);
			if (role != "Mafia" && role != "Kil") role = "Innocent";
			kom_id->ich->send_message("Player " + std::to_string(cur_action->will_be_checked) + " is " 
												+ role + "\n");
			logger.logRound(day_counter, "Kom checks "+ 
									std::to_string(cur_action->will_be_checked) + " is " 
												+ role);
			stat.kom_checks++;
		}
		if (cur_action->will_be_blessed != -1){
			Player* tmp = will_be_checked_by_pri.get();
			std::string role = std::string(typeid(*(tmp)).name()).substr(1);
			if (role == "Player") role = "Innocent";
			pri_id->ich->send_message("Player " + std::to_string(cur_action->will_be_blessed) + " is " 
												+ role + "\n");
			will_be_checked_by_pri->ich->send_message("Player " + std::to_string(pri_id->ich->id) + " is Priest\n");
			logger.logRound(day_counter, "Priest checks "+ 
									std::to_string(cur_action->will_be_blessed) + " is " 
												+ role);
			stat.priest_checks++;

		}
		if (killed.contains(cur_action->will_be_extrakilled))
			stat.mafia_kills++;
		if (killed.contains(cur_action->will_be_killed))
			stat.kom_kills++;
		if (killed.contains(cur_action->will_be_killed_byman))
			stat.man_kills++;
		if (mostFrequentElement(cur_action->mafia_votes, cur_action->don_choice))
			stat.mafia_kills++;


		

		std::ranges::for_each(players, [&killed](SmartPointer<Player> pl)
				{ if (killed.contains(pl->ich->id)) { 
						pl->alive = false;
				} });	
	}

	void in_lobby(){
		num_of_ready = 0;
		while (num_of_ready < MIN_PALYERS || num_of_ready < clients.size()){
				fd_set read_fds;
				FD_ZERO(&read_fds);

        		// Добавляем серверный сокет в набор для select
        		FD_SET(server_fd, &read_fds);

        		// Добавляем все клиентские сокеты в набор для select
        		for (auto client_fd : clients) {
        		    FD_SET(client_fd->client_socket, &read_fds);
        		    if (client_fd->client_socket > max_sd)
        		        max_sd = client_fd->client_socket;
        		}

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
						clients.push_back(SmartPointer<Client>(new Client(new_socket)));
        		        std::cout << "New client connected: " << clients.back()->id << "\n";
						set_non_blocking(new_socket);
						clients.back()->send_message("Press y to get ready\nSend disconnect to disconnect\n");
        		    }
        		}

        		// Обрабатываем данные от каждого клиента
        		for (auto it = clients.begin(); it != clients.end(); ) {
					if (not(it->get()->active)){
						it = clients.erase(it); // Erase returns the next iterator
				        continue;
					}
						
				    int& client_fd = it->get()->client_socket; 						
				    if (FD_ISSET(client_fd, &read_fds)) {
				        // Reading data from the client
				        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);
						
						if (bytes_read <= 0){
							it++;
				            continue;
						} else
							buffer[bytes_read] = '\0';
						if (buffer[0] == 'y' && !it->get()->ready) {
				            it->get()->send_message("You are ready!");
				            num_of_ready++;
							std::cout << "Ready players: " << num_of_ready <<"\n";
				            it->get()->ready = true;
						} 
						else if (std::string(buffer) == "list") {
							std::string mes = "Current clients:";
							for (auto el: clients)
								mes += " " + std::to_string(el->id);
							it->get()->send_message(mes);
						
						}
						else if (std::string(buffer) == "disconnect"){
							std::cout << "Client disconnected: " << std::to_string(it->get()->id) << "\n";
				            close(client_fd);
				            it = clients.erase(it); // Erase returns the next iterator	
						}
						else {
							std::string chat_message = " [Global] "+ std::to_string(it->get()->id) + ": "+ std::string(buffer);
							send_all(chat_message);
						}
						memset(buffer, 0, sizeof(buffer));
						
				    } else {
				        ++it;
					}
				}

				
        		if (activity == 0 and num_of_ready >= MIN_PALYERS) {
					
					break;
        		}
		}
		for (auto it = clients.begin(); it != clients.end(); ){
			if (not it->get()->ready){
				std::cout << "Client disconnected: " << std::to_string(it->get()->id) << "\n";
				close(it->get()->client_socket);
				it = clients.erase(it); 
			}
			else it++;
		}
		std::cout << "Game has been started!\n";
		send_all("[Announcement] Game has been started! Everybody who is not ready --- it's yours problem");
	}

	std::pair<bool, std::string> check_endgame(){
		int mafia_num = 0;
		bool man_live = false;
		int alive = 0;
		std::ranges::for_each(players, [&alive, &mafia_num, &man_live](SmartPointer<Player> pl)
				{ if (pl->alive){
					alive++;
					Player* tmp = pl.get();
					std::string role = std::string(typeid(*(tmp)).name()).substr(1);
					if (role == "Mafia" || role == "Kil")
						mafia_num++;
					if (role == "Man") man_live = true;
					}
				});
		if (mafia_num*2 >= alive && !man_live) return std::pair<bool, std::string>(true, "Mafia");
		if (mafia_num == 0 && !man_live) return std::pair<bool, std::string>(true, "Innocent");
		if (alive == 1 && man_live) return std::pair<bool, std::string>(true, " Legengary maniac");
		if (alive == 0) return std::pair<bool, std::string>(true, "Friendship");
		return std::pair<bool, std::string>(false, "");;

	}

    
};





int main() {


	Game game(PORT);
	game.run();
	std::cout << "End\n";
    return 0;
}

