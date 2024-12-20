#include "main.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Необходимо указать количество потоков, количество задач и количество процессоров." << endl;
        return 1;
    }

    int num_threads = stoi(argv[1]);
    int N = stoi(argv[2]);
    int M = stoi(argv[3]);
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

    Solution* best_solution = sa.getBestSolution();
    cout << "К1: " << best_solution->calculateCost() << endl;

    ofstream results_file("results_parallel.csv", ios::app);
    if (results_file.is_open()) {
        results_file << num_threads << "," << N << "," << M << "," << elapsed.count() << "," << best_solution->calculateCost() << endl;
        results_file.close();
    } else {
        cerr << "Не удалось открыть файл для записи." << endl;
    }

    delete boltzmann;
    delete mutation;
    delete best_solution;
    delete initial_solution;

    return 0;
}

