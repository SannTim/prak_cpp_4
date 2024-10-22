#include "../main.h"
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#define VERBOSE_L true
using namespace std;
using namespace chrono;

std::mutex mtx;

ScheduleSolution solve_parallel(CoolingSchedule* col, Mutation* mut, ScheduleSolution* initial_solution) {
    SimulatedAnnealing sa(col, mut);
    sa.run(initial_solution);
	ScheduleSolution& sched = static_cast<ScheduleSolution&>(*sa.getBestSolution());
	return sched;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Необходимо указать количество потоков." << endl;
        return 1;
    }

    int num_threads = stoi(argv[1]);
    int N = stoi(argv[2]);  
    int M = stoi(argv[3]);  
    vector<int> jobs = generateJobs(N);  

    auto boltzmann = make_unique<BoltzmannCooling>(1000.0);
    auto mutation = make_unique<ScheduleMutation>();
    auto initial_solution = make_unique<ScheduleSolution>(N, M, jobs);
    auto best_global = make_unique<ScheduleSolution>(N, M, jobs);

    for (int i = 0; i < N; ++i) {
        initial_solution->schedule[i][i % M] = true;
        best_global->schedule[i][i % M] = true;
    }

    auto start = high_resolution_clock::now();

    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&mutation, &boltzmann, &initial_solution, &best_global, i]() {
            ScheduleSolution best_local = solve_parallel(boltzmann.get(), mutation.get(), initial_solution.get());
			lock_guard<mutex> lock(mtx);
			if (VERBOSE_L){
				cout << "Best local "<< i << ": " << endl;
				best_local.print();
				cout << "K: " << best_local.calculateCost() << endl;
			}
            if (best_local.calculateCost() < best_global->calculateCost()) {
                *best_global = std::move(best_local);
            }
        });
    }   

    for (auto& thread : threads) {
        thread.join();
    } 

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    cout << "Время выполнения: " << elapsed.count() << " секунд." << endl;

    cout << "Окончательное расписание:" << endl;
    best_global->print();
	cout << "K: " << best_global->calculateCost() << endl;

    return 0;
}

