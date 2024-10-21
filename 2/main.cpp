#include "main.h"
#include <iostream>

int main() {
    int N = 7; 
	int M = 3;  
	vector<int> jobs = {5, 2, 3, 7, 1, 8, 6};

    CoolingSchedule* boltzmann = new BoltzmannCooling(1000.0);
    CoolingSchedule* cauchy = new CauchyCooling(1000.0);
    CoolingSchedule* log_cauchy = new LogarithmicCauchyCooling(1000.0);


    Mutation* mutation = new ScheduleMutation();


    ScheduleSolution* initial_solution = new ScheduleSolution(N, M, jobs);
    for (int i = 0; i < N; ++i) {
        initial_solution->schedule[i][i % M] = true;
    }

    SimulatedAnnealing sa(boltzmann, mutation); 

    int num_threads = 4;
    vector<thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(thread(&SimulatedAnnealing::run, &sa, initial_solution, 1000, i));
    }


    for (auto& t : threads) {
        t.join();
    }


    Solution* best_solution = sa.getBestSolution();
    best_solution->print();
    cout << "Best cost: " << best_solution->calculateCost() << endl;

    delete best_solution;
    delete initial_solution;
    delete boltzmann;
    delete cauchy;
    delete log_cauchy;
    delete mutation;

    return 0;
}


