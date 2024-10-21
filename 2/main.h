#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <limits>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

using namespace std;

class Solution {
public:
    virtual int calculateCost() = 0;  
    virtual void print() const = 0;   
	virtual Solution* clone() const = 0;
	virtual ~Solution() = default;
};

class Mutation {
public:
	virtual ~Mutation() = default;
    virtual void mutate(Solution& solution) = 0; 
};

class CoolingSchedule {
public:
    virtual ~CoolingSchedule() = default;
    virtual double getTemperature() const = 0;  // Получить текущую температуру
    virtual void cool(int iteration) = 0;       // Охладить, учитывая номер итерации
};

class ScheduleMutation : public Mutation {
public:
    void mutate(Solution& solution) override;
};

class ScheduleSolution : public Solution {
public:
    vector<vector<bool>> schedule;
    vector<int> job_times;

    ScheduleSolution(int N, int M, vector<int>& jobs) : job_times(jobs) {
        schedule.resize(N, vector<bool>(M, false));
    }
	ScheduleSolution(int N, int M, const vector<int>& jobs) : job_times(jobs) {
        schedule.resize(N, vector<bool>(M, false));
    }

    int calculateCost() override {
        vector<int> completion_times(schedule[0].size(), 0);
        for (int j = 0; j < job_times.size(); ++j) {
            for (int p = 0; p < schedule[j].size(); ++p) {
                if (schedule[j][p]) {
                    completion_times[p] += job_times[j];
                    break;
                }
            }
        }
        int Tmax = *max_element(completion_times.begin(), completion_times.end());
        int Tmin = *min_element(completion_times.begin(), completion_times.end());
        return Tmax - Tmin;
    }

    void print() const override {
        for (int j = 0; j < schedule.size(); ++j) {
            cout << "Job " << j << ": ";
            for (int p = 0; p < schedule[j].size(); ++p) {
                cout << (schedule[j][p] ? "1 " : "0 ");
            }
            cout << endl;
        }
    }

    Solution* clone() const override {
        return new ScheduleSolution(*this);
    }
};

void ScheduleMutation::mutate(Solution& solution) {
    ScheduleSolution& sched = static_cast<ScheduleSolution&>(solution);
    int N = sched.schedule.size();
    int M = sched.schedule[0].size();

    int job_idx = rand() % N;
    int new_processor = rand() % M;

    for (int p = 0; p < M; ++p) {
        sched.schedule[job_idx][p] = (p == new_processor);
    }
}


class BoltzmannCooling : public CoolingSchedule {
    double initial_temp;
    double temperature;
public:
    BoltzmannCooling(double temp) : initial_temp(temp), temperature(temp) {}

    double getTemperature() const override {
        return temperature;
    }

    void cool(int iteration) override {
        temperature = initial_temp / log(1 + iteration);  // T = T0 / ln(1 + i)
    }
};

class CauchyCooling : public CoolingSchedule {
    double initial_temp;
    double temperature;
public:
    CauchyCooling(double temp) : initial_temp(temp), temperature(temp) {}

    double getTemperature() const override {
        return temperature;
    }

    void cool(int iteration) override {
        temperature = initial_temp / (1 + iteration);  // T = T0 / (1 + i)
    }
};

class LogarithmicCauchyCooling : public CoolingSchedule {
    double initial_temp;
    double temperature;
public:
    LogarithmicCauchyCooling(double temp) : initial_temp(temp), temperature(temp) {}

    double getTemperature() const override {
        return temperature;
    }

    void cool(int iteration) override {
        temperature = initial_temp * (log(1 + iteration) / (1 + iteration));  // T = T0 * (ln(1 + i) / (1 + i))
    }
};

class SimulatedAnnealing {
    CoolingSchedule* cooling;
    Mutation* mutation;
    mutex mtx;
    Solution* global_best_solution;
    int best_cost;

public:
    SimulatedAnnealing(CoolingSchedule* cooling, Mutation* mutation)
        : cooling(cooling), mutation(mutation), global_best_solution(nullptr), best_cost(numeric_limits<int>::max()) {}

    void run(Solution* initial_solution, int max_iterations, int thread_id) {
        Solution* current_solution = initial_solution->clone();
        Solution* best_solution = initial_solution->clone();
        int current_cost = current_solution->calculateCost();
        int best_cost_local = current_cost;

        for (int i = 0; i < max_iterations; ++i) {
            Solution* new_solution = current_solution->clone();
            mutation->mutate(*new_solution);

            int new_cost = new_solution->calculateCost();
            if (new_cost < current_cost || exp((current_cost - new_cost) / cooling->getTemperature()) > ((double) rand() / RAND_MAX)) {
                delete current_solution;
                current_solution = new_solution;
                current_cost = new_cost;
            } else {
                delete new_solution;
            }

            if (current_cost < best_cost_local) {
                delete best_solution;
                best_solution = current_solution->clone();
                best_cost_local = current_cost;
            }

            cooling->cool(i + 1);

            {
                lock_guard<mutex> lock(mtx);
                if (best_cost_local < best_cost) {
                    best_cost = best_cost_local;
                    delete global_best_solution;
                    global_best_solution = best_solution->clone();
                }
            }
        }

        delete current_solution;
        delete best_solution;
    }

    Solution* getBestSolution() {
        lock_guard<mutex> lock(mtx);
        return global_best_solution ? global_best_solution->clone() : nullptr;
    }
};

vector<int> generateJobs(int N) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 100);

    vector<int> jobs(N);
    for (int i = 0; i < N; ++i) {
        jobs[i] = dis(gen);
    }
    return jobs;
}


class ParallelSimulatedAnnealing {
    vector<thread> workers;
    mutex mtx;  // Мьютекс для синхронизации
    Solution* global_best_solution;  // Глобальное лучшее решение
    int global_best_cost;
    bool improved;  // Флаг для проверки улучшения решения
    int num_iterations_no_improve;

public:
    ParallelSimulatedAnnealing() 
        : global_best_solution(nullptr), global_best_cost(numeric_limits<int>::max()), improved(false), num_iterations_no_improve(0) {}

    ~ParallelSimulatedAnnealing() {
        if (global_best_solution) {
            delete global_best_solution;
        }
    }

    void run_parallel(int num_threads, int max_iterations, Solution* initial_solution, vector<CoolingSchedule*>& cooling_schedules, Mutation* mutation) {
        // Убедимся, что очищаем вектор потоков перед запуском нового набора
        clear_workers();
        
        for (int i = 0; i < num_threads; ++i) {
            workers.emplace_back([this, initial_solution, max_iterations, i, &cooling_schedules, mutation]() {
                this->run_single_instance(initial_solution->clone(), max_iterations, i, cooling_schedules[i], mutation);
            });
        }

        // Ждем завершения всех потоков
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }

        clear_workers();  // Очищаем после завершения работы
    }

    void run_single_instance(Solution* initial_solution, int max_iterations, int thread_id, CoolingSchedule* cooling_schedule, Mutation* mutation) {
        Solution* current_solution = initial_solution->clone();
        Solution* best_solution = initial_solution->clone();
        int current_cost = current_solution->calculateCost();
        int best_cost = current_cost;

        for (int i = 0; i < max_iterations; ++i) {
            Solution* new_solution = current_solution->clone();
            mutation->mutate(*new_solution);

            int new_cost = new_solution->calculateCost();
            if (new_cost < current_cost || exp((current_cost - new_cost) / cooling_schedule->getTemperature()) > ((double) rand() / RAND_MAX)) {
                delete current_solution;
                current_solution = new_solution;
                current_cost = new_cost;
            } else {
                delete new_solution;
            }

            if (current_cost < best_cost) {
                delete best_solution;
                best_solution = current_solution->clone();
                best_cost = current_cost;
            }

            cooling_schedule->cool(i + 1);

            lock_guard<mutex> lock(mtx);

            if (best_cost < global_best_cost) {
                global_best_cost = best_cost;
                if (global_best_solution) {
                    delete global_best_solution;
                }
                global_best_solution = best_solution->clone();
                improved = true;
                num_iterations_no_improve = 0;  // Обнуляем счетчик итераций без улучшений
            }
        }

        delete current_solution;
        delete best_solution;
    }

    bool has_converged() {
        return num_iterations_no_improve >= 10;  // 10 итераций без улучшений
    }

    Solution* get_best_solution() {
        lock_guard<mutex> lock(mtx);
        return global_best_solution ? global_best_solution->clone() : nullptr;
    }

    int get_best_cost() {
        lock_guard<mutex> lock(mtx);
        return global_best_cost;
    }

    void reset_improvement_flag() {
        improved = false;
    }

    bool is_improved() {
        return improved;
    }

    void increment_no_improve() {
        if (!improved) {
            ++num_iterations_no_improve;
        }
    }

    // Метод для очистки вектора потоков
    void clear_workers() {
        workers.clear();
    }
};
