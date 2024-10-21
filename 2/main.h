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

