#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include "main.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <memory>
#include <limits>
#include <thread>
#define CUR_M 50

using namespace std;
using namespace chrono;

int main() {
    ofstream time_file("parallel_times.csv");
    ofstream accuracy_file("parallel_accuracy.csv");

    time_file << "N,num_threads,time\n";
    accuracy_file << "N,num_threads,accuracy\n";

    int M = 50;
    vector<int> test_N = {250, 500, 750, 1000, 1250};
    vector<int> thread_counts = {1, 2, 3, 4};

    for (int N : test_N) {
        vector<int> jobs = generateJobs(N);

        for (int num_threads : thread_counts) {
            ParallelSimulatedAnnealing parallel_sa;

            vector<CoolingSchedule*> cooling_schedules(num_threads);
            for (int i = 0; i < num_threads; ++i) {
                cooling_schedules[i] = new BoltzmannCooling(1000.0);  // Используем Болцмановское охлаждение для всех потоков
            }

            Mutation* mutation = new ScheduleMutation();
            ScheduleSolution* initial_solution = new ScheduleSolution(N, M, jobs);

            auto start = high_resolution_clock::now();

            for (int iteration = 0; !parallel_sa.has_converged() && iteration < 10; ++iteration) {
                parallel_sa.run_parallel(num_threads, 1000, initial_solution, cooling_schedules, mutation);

                parallel_sa.increment_no_improve();
                parallel_sa.reset_improvement_flag();
            }

            auto end = high_resolution_clock::now();
            duration<double> elapsed = end - start;
            double elapsed_time = elapsed.count();

            Solution* best_solution = parallel_sa.get_best_solution();
            int best_cost = parallel_sa.get_best_cost();

            time_file << N << "," << num_threads << "," << elapsed_time << "\n";
            accuracy_file << N << "," << num_threads << "," << best_cost << "\n";

            cout << "N: " << N << ", Threads: " << num_threads << ", Time: " << elapsed_time << " sec, Best Cost: " << best_cost << endl;

            // Освобождаем память
            delete initial_solution;
            delete mutation;
            for (CoolingSchedule* cs : cooling_schedules) {
                delete cs;
            }
            delete best_solution;

            // Очищаем вектор потоков после завершения работы
            parallel_sa.clear_workers();
        }
    }

    time_file.close();
    accuracy_file.close();

    return 0;
}
