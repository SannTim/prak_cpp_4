#include "../main.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Необходимо указать количество потоков." << endl;
        return 1;
    }

    int num_threads = stoi(argv[1]);
    int N = 100000;  
    int M = 5000;  
    vector<int> jobs = generateJobs(N);  

    CoolingSchedule* boltzmann = new BoltzmannCooling(1000.0);
    Mutation* mutation = new ScheduleMutation();
    ScheduleSolution* initial_solution = new ScheduleSolution(N, M, jobs);
    
    for (int i = 0; i < N; ++i) {
        initial_solution->schedule[i][i % M] = true;
    }

    SimulatedAnnealing sa(boltzmann, mutation);
    
    auto start = high_resolution_clock::now();

    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&sa, initial_solution]() {
            sa.run(initial_solution);
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    cout << "Время выполнения: " << elapsed.count() << " секунд." << endl;

    // cout << "Окончательное расписание:" << endl;
    Solution* best_solution = sa.getBestSolution();
    // best_solution->print();
    cout << "К1: " << best_solution->calculateCost() << endl;

    delete boltzmann;
    delete mutation;
    delete best_solution;
    delete initial_solution;

    return 0;
}

