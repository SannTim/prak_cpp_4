#include "GeneticAlgorithm.hpp"
#include "CellularAutomaton.hpp"
#include <iostream>
#include <fstream>
#include <cmath>

int main() {
    const int rows = 50;
    const int cols = 50;
    const int populationSize = 100;
    const double crossoverRate = 0.8;
    const int maxGenerationsWithoutImprovement = 50;
    const int generations = 100;

    const double initialMutationRate = 0.0004;
    const double mutationScaleFactor = 1.5;

    std::cout << "Starting Genetic Algorithm Simulation...\n";
    std::cout << "Grid size: " << rows << "x" << cols << "\n";
    std::cout << "Population size: " << populationSize << "\n";
    std::cout << "Crossover rate: " << crossoverRate << "\n";
    std::cout << "Max generations without improvement: " << maxGenerationsWithoutImprovement << "\n";
    std::cout << "Generations to evolve cellular automaton: " << generations << "\n";

    for (int i = 0; i <= 9; ++i) {
        double mutationRate = initialMutationRate * std::pow(mutationScaleFactor, i);
        std::cout << "\n--- Mutation Rate Series " << i << ": " << mutationRate << " ---\n";

        for (int run = 0; run < 10; ++run) {
            std::cout << "  Run " << run << ":\n";

            GeneticAlgorithm ga(populationSize, crossoverRate, maxGenerationsWithoutImprovement, rows);
            std::cout << "    Initializing Genetic Algorithm...\n";

            ga.run(mutationRate, true);
            std::cout << "    Genetic Algorithm run completed.\n";
            std::cout << "    Best solution fitness: " << ga.getBestFitness() << "\n";

            std::string filename = "series_" + std::to_string(i) + "_run_" + std::to_string(run) + "_sol.txt";
            std::ofstream file(filename);
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c) {
                    file << (ga.getBestSolution()[r * cols + c] ? "X" : "-");
                }
                file << '\n';
            }
            std::cout << "    Best solution saved to " << filename << "\n";

            CellularAutomaton ca(rows, cols, ga.getBestSolution());
            ca.evolve(generations);
            std::cout << "    Cellular Automaton evolved for " << generations << " generations.\n";

            std::string after100Filename = "series_" + std::to_string(i) + "_run_" + std::to_string(run) + "_sol_after100.txt";
            std::ofstream after100File(after100Filename);
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c) {
                    after100File << (ca.getCurrentState()[r][c] ? "X" : "-");
                }
                after100File << '\n';
            }
            std::cout << "    Solution after 100 generations saved to " << after100Filename << "\n";
        }
    }

    std::cout << "\nSimulation completed.\n";
    return 0;
}

