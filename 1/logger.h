#include <filesystem>
#include <string>
#include <fstream>

// Логирование
class Logger {
    std::filesystem::path dir;

public:
    Logger() {
        dir = std::filesystem::current_path() / "logs";
        std::filesystem::create_directory(dir);
		int gamenum = 0;
		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			gamenum++;
		}
		dir = dir / std::to_string(gamenum);
		std::filesystem::create_directory(dir);
    }

    void logRound(int round, const std::string& log) {
        std::ofstream log_file(dir / ("Round_" + std::to_string(round) + ".txt"), std::ios::app);
        log_file << log << "\n";
        log_file.close();
    }

    void logFinal(const std::string& final_log) {
        std::ofstream final_file(dir / "Statistic.log", std::ios::app);
        final_file << final_log << "\n";
        final_file.close();
    }
};
