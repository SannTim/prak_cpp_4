#include <__config>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <limits>
#include "main.h"
#define STEP 100
#define TIME_END 60
#define MAX_M 80
#define M_STEP 12
using namespace std;
using namespace chrono;



double Simulate(int N, int M, CoolingSchedule* algo, const vector<int>& jobs, ScheduleSolution*& final_solution) {
    Mutation* mutation = new ScheduleMutation();
    ScheduleSolution* initial_solution = new ScheduleSolution(N, M, jobs);

    for (int i = 0; i < N; ++i) {
        initial_solution->schedule[i][i % M] = true;
    }

    SimulatedAnnealing sa(algo, mutation);
    auto start = high_resolution_clock::now();
    
    sa.run(initial_solution, 100000, 0);
    final_solution = initial_solution;

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;

    delete mutation;
    return elapsed.count();
}

int main() {
    ofstream results_file("results_alg.csv");
    results_file << "N,M,boltzmann_time,cauchy_time,log_cauchy_time" << endl;

    int N = STEP;
    double avg_boltzmann = 0, avg_cauchy = 0, avg_log_cauchy = 0;
    int num_experiments = 0;

    while (true) {
		double average_time; 
        for (int M = 2; M < MAX_M; M+=M_STEP){

			// Генерация одинаковых наборов работ для всех законов охлаждения
        	vector<int> jobs = generateJobs(N);

        	CoolingSchedule* boltzmann = new BoltzmannCooling(1000.0);
        	CoolingSchedule* cauchy = new CauchyCooling(1000.0);
        	CoolingSchedule* log_cauchy = new LogarithmicCauchyCooling(1000.0);
			
        	// Запускаем симуляцию для каждого закона охлаждения
			ScheduleSolution* final_boltzmann_solution = nullptr;
            ScheduleSolution* final_cauchy_solution = nullptr;
            ScheduleSolution* final_log_cauchy_solution = nullptr;

            double boltzmann_time = Simulate(N, M, boltzmann, jobs, final_boltzmann_solution);
            double cauchy_time = Simulate(N, M, cauchy, jobs, final_cauchy_solution);
            double log_cauchy_time = Simulate(N, M, log_cauchy, jobs, final_log_cauchy_solution);
	
				

        	// Запись результатов в файл
        	results_file << N << "," << M << "," << boltzmann_time << "," << cauchy_time << "," << log_cauchy_time << endl;

        	// Обновление среднего времени для каждого закона охлаждения
        	avg_boltzmann += boltzmann_time;
        	avg_cauchy += cauchy_time;
        	avg_log_cauchy += log_cauchy_time;
        	num_experiments++;

        	cout << "N: " << N << " M: " << M << endl;
        	cout << "Boltzmann: " << boltzmann_time << " seconds" << endl;
        	cout << "Cauchy: " << cauchy_time << " seconds" << endl;
        	cout << "Log Cauchy: " << log_cauchy_time << " seconds" << endl;

			if (final_boltzmann_solution) {
                cout << "Boltzmann schedule at N = " << N << ", M = " << M << ":\n";
                final_boltzmann_solution->print();
                delete final_boltzmann_solution;  // Clean up after printing
            }

        	delete boltzmann;
        	delete cauchy;
        	delete log_cauchy;

        	average_time = (boltzmann_time + cauchy_time + log_cauchy_time) / 3.0;

        	

		}
		if (average_time > TIME_END) {
           break;
        }
		N += STEP; 
    }

    // Рассчет среднего времени
    avg_boltzmann /= num_experiments;
    avg_cauchy /= num_experiments;
    avg_log_cauchy /= num_experiments;

    cout << "Average Boltzmann time: " << avg_boltzmann << " seconds" << endl;
    cout << "Average Cauchy time: " << avg_cauchy << " seconds" << endl;
    cout << "Average Log Cauchy time: " << avg_log_cauchy << " seconds" << endl;

    // Определение самого медленного закона охлаждения
    if (avg_boltzmann > avg_cauchy && avg_boltzmann > avg_log_cauchy) {
        cout << "Boltzmann is the slowest." << endl;
    } else if (avg_cauchy > avg_boltzmann && avg_cauchy > avg_log_cauchy) {
        cout << "Cauchy is the slowest." << endl;
    } else {
        cout << "Logarithmic Cauchy is the slowest." << endl;
    }

    results_file.close();

    return 0;
}

