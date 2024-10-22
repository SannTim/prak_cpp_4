#include "../main.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace std;
using namespace chrono;

int main() {
    int N = 1000;  // Количество задач
    int M = 300;  // Количество процессоров
    vector<int> jobs = generateJobs(1000);  // Пример длительности выполнения задач

    CoolingSchedule* boltzmann = new BoltzmannCooling(1000.0);
    Mutation* mutation = new ScheduleMutation();

    ScheduleSolution* initial_solution = new ScheduleSolution(N, M, jobs);
    for (int i = 0; i < N; ++i) {
        initial_solution->schedule[i][i % M] = true;
    }

    SimulatedAnnealing sa(boltzmann, mutation);

    auto start = high_resolution_clock::now();

    sa.run(initial_solution);

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    cout << "Время выполнения: " << elapsed.count() << " секунд." << endl;

    cout << "Окончательное расписание:" << endl;
    Solution* best_solution = sa.getBestSolution();
    best_solution->print();
    cout << "К1: " << best_solution->calculateCost() << endl;

    delete boltzmann;
    delete mutation;
    delete best_solution;
    delete initial_solution;

    return 0;
}

