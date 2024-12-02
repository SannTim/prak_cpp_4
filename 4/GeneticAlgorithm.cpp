#include "GeneticAlgorithm.hpp"
#include "CellularAutomaton.hpp"
#include <algorithm>
#include <iostream>
#include <random>

GeneticAlgorithm::GeneticAlgorithm(int populationSize, double crossoverRate, int maxGenerationsWithoutImprovement, int gridSize)
    : populationSize(populationSize), crossoverRate(crossoverRate), maxGenerationsWithoutImprovement(maxGenerationsWithoutImprovement), gridSize(gridSize), bestFitness(-1) {
    std::random_device rd;
    rng = std::mt19937(rd());
}

void GeneticAlgorithm::initializePopulation() {
    population.resize(populationSize);
    std::uniform_int_distribution<int> dist(0, 1);

    for (int i = 0; i < populationSize; ++i) {
        std::vector<int> individual(gridSize * gridSize);
        for (int j = 0; j < gridSize * gridSize; ++j) {
            individual[j] = dist(rng);
        }
        population[i] = individual;
    }
}

void GeneticAlgorithm::evaluateFitness() {
    bestFitness = -1;

    for (const auto& individual : population) {
        CellularAutomaton automaton(decodeGrid(individual));
        automaton.evolve(100); // Эволюция на 100 поколений

        int fitness = countFilledCells(automaton.getCurrentState());

        if (fitness > bestFitness) {
            bestFitness = fitness;
            bestSolution = individual;
        }
    }
}

int GeneticAlgorithm::countFilledCells(const std::vector<std::vector<int>>& grid) const {
    int filledCells = 0;
    for (const auto& row : grid) {
        filledCells += std::count(row.begin(), row.end(), 1);
    }
    return filledCells;
}

std::vector<std::vector<int>> GeneticAlgorithm::decodeGrid(const std::vector<int>& individual) const {
    std::vector<std::vector<int>> grid(gridSize, std::vector<int>(gridSize));
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            grid[i][j] = individual[i * gridSize + j];
        }
    }
    return grid;
}

void GeneticAlgorithm::selection() {
    // Выбор двух родителей из популяции
    std::uniform_int_distribution<int> dist(0, populationSize - 1);

    int idx1 = dist(rng);
    int idx2 = dist(rng);

    CellularAutomaton automaton1(decodeGrid(population[idx1]));
    automaton1.evolve(50); // Эволюция на 50 поколений
    int fitness1 = countFilledCells(automaton1.getCurrentState());

    CellularAutomaton automaton2(decodeGrid(population[idx2]));
    automaton2.evolve(50);
    int fitness2 = countFilledCells(automaton2.getCurrentState());

    if (fitness2 > fitness1) {
        std::swap(population[idx1], population[idx2]);
    }
}

void GeneticAlgorithm::crossover() {
    for (int i = 0; i < populationSize; i += 2) {
        if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) < crossoverRate) {
            onePointCrossover(population[i], population[i + 1]);
        }
    }
}

void GeneticAlgorithm::mutate(double mutationRate) {
    for (auto& individual : population) {
        for (auto& gene : individual) {
            if (std::uniform_real_distribution<double>(0.0, 1.0)(rng) < mutationRate) {
                gene = 1 - gene;  // Инвертируем бит
            }
        }
    }
}

void GeneticAlgorithm::onePointCrossover(std::vector<int>& parent1, std::vector<int>& parent2) {
    std::uniform_int_distribution<int> dist(1, parent1.size() - 1);
    int crossoverPoint = dist(rng);

    std::vector<int> offspring1(parent1.begin(), parent1.begin() + crossoverPoint);
    offspring1.insert(offspring1.end(), parent2.begin() + crossoverPoint, parent2.end());

    std::vector<int> offspring2(parent2.begin(), parent2.begin() + crossoverPoint);
    offspring2.insert(offspring2.end(), parent1.begin() + crossoverPoint, parent1.end());

    parent1 = offspring1;
    parent2 = offspring2;
}

void GeneticAlgorithm::run(double mutationRate, bool verbose) {
    initializePopulation();
    int generationsWithoutImprovement = 0;

    evaluateFitness();

    while (generationsWithoutImprovement < maxGenerationsWithoutImprovement) {
        int previousBestFitness = bestFitness;

        selection();
        crossover();
        mutate(mutationRate);

        evaluateFitness();

        if (bestFitness > previousBestFitness) {
            generationsWithoutImprovement = 0;
        } else {
            ++generationsWithoutImprovement;
        }

        if (verbose) {
            std::cout << "Best Fitness: " << bestFitness << std::endl;
        }
    }
}

const std::vector<int>& GeneticAlgorithm::getBestSolution() const {
    return bestSolution;
}

int GeneticAlgorithm::getBestFitness() {
    return bestFitness;
}

