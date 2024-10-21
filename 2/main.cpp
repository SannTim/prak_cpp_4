#include "main.h"
#include <vector>
#include <iostream>


int main() {
    // Параметры
    int N = 7;  // Количество работ
    int M = 3;  // Количество процессоров
    vector<int> jobs = {5, 2, 3, 7, 1, 8, 6};  // Время выполнения работ

    // Создаем законы понижения температуры
    CoolingSchedule* linear = new LinearCooling(1000.0, 10.0);
    CoolingSchedule* exp = new ExponentialCooling(1000.0, 0.95);
    CoolingSchedule* logar = new LogarithmicCooling(1000.0, 0.1);

    // Создаем мутацию
    Mutation* mutation = new ScheduleMutation();

    // Создаем начальное решение
    ScheduleSolution* initial_solution = new ScheduleSolution(N, M, jobs);
    for (int i = 0; i < N; ++i) {
        initial_solution->schedule[i][i % M] = true;
    }

    // Параллельная реализация
    SimulatedAnnealing sa(linear, mutation);

    // Запускаем параллельно несколько потоков
    int num_threads = 4;
    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread(&SimulatedAnnealing::run, &sa, initial_solution, 1000, i));
    }

    // Ожидаем завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    // Получаем и выводим лучшее решение
    Solution* best_solution = sa.getBestSolution();
    best_solution->print();
    cout << "Best cost: " << best_solution->calculateCost() << endl;

    delete best_solution;
    delete initial_solution;
    delete linear;
    delete exp;
    delete logar;
    delete mutation;

    return 0;
}

