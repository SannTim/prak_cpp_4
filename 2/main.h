#include <algorithm>
#include <climits>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <limits>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#define VERBOSE false
#define OPPOSITE false
#define NO_IMPOVE 10
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
    virtual double getTemperature() const = 0;
    virtual void cool(int iteration) = 0;
};

class ScheduleMutation : public Mutation {
public:
    ScheduleMutation();
    void mutate(Solution& solution) override;

private:
    std::mt19937 gen;
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
		if (OPPOSITE)
			return INT_MAX - (Tmax - Tmin);
		else return Tmax - Tmin;
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

ScheduleMutation::ScheduleMutation()
    : gen(std::random_device{}())
{}

void ScheduleMutation::mutate(Solution& solution) {
    ScheduleSolution& sched = static_cast<ScheduleSolution&>(solution);
    int N = sched.schedule.size();
    int M = sched.schedule[0].size();

    std::uniform_int_distribution<> job_dist(0, N - 1);
    std::uniform_int_distribution<> proc_dist(0, M - 1);

    int job_idx = job_dist(gen);
    int new_processor = proc_dist(gen);

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
        temperature = initial_temp / log(1 + iteration);
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
        temperature = initial_temp / (1 + iteration);
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
        temperature = initial_temp * (log(1 + iteration) / (1 + iteration));
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

class SimulatedAnnealing {
private:
    CoolingSchedule* cooling;
    Mutation* mutation;
    mutex mtx;
    Solution* global_best_solution;
    int best_cost;
	int no_improvement_count;

public:
    SimulatedAnnealing(CoolingSchedule* cooling, Mutation* mutation)
        : cooling(cooling), mutation(mutation), global_best_solution(nullptr), best_cost(numeric_limits<int>::max()) {
			no_improvement_count = 0;
		}

    void run(Solution* initial_solution) {
		Solution* current_solution = initial_solution->clone();
		int current_cost = current_solution->calculateCost();
    	
    	for (int i = 0; no_improvement_count < NO_IMPOVE; ++i) {
    	    Solution* new_solution = current_solution->clone();
    	    mutation->mutate(*new_solution);
			
    	    int new_cost = new_solution->calculateCost();
    	    double temperature = cooling->getTemperature();
    	    if (VERBOSE) std::cout << "Bc: " << best_cost << ", K1: " << new_cost << "\n";
    	    double acceptance_probability = exp((current_cost - new_cost) / temperature);
    	    if (new_cost < current_cost || acceptance_probability > ((double)rand() / RAND_MAX)) {
				if (VERBOSE) std::cout << "Ac: " << acceptance_probability << "Rand" << ((double)rand() / RAND_MAX) << "\n";
    	        delete current_solution;
    	        current_solution = new_solution;
    	        current_cost = new_cost;
    	    } else {
    	        delete new_solution;
    	    }

    	    cooling->cool(i + 1);

    	    {
    	        lock_guard<mutex> lock(mtx);
    	        if (current_cost < best_cost) {
    	            best_cost = current_cost;
    	            if (global_best_solution) {
    	                delete global_best_solution;
    	            }
    	            global_best_solution = current_solution->clone();
    	            no_improvement_count = 0;  // Сбросить счетчик улучшений
    	        } else no_improvement_count++;

    	    }
    	}

    	delete current_solution;
    }

    Solution* getBestSolution() {
        lock_guard<mutex> lock(mtx);
        return global_best_solution ? global_best_solution->clone() : nullptr;
    }
};

