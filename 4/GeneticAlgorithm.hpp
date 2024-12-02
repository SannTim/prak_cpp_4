#ifndef GENETICALGORITHM_HPP
#define GENETICALGORITHM_HPP

#include <vector>
#include <random>

class GeneticAlgorithm {
public:
    GeneticAlgorithm(int populationSize, double crossoverRate, int maxGenerationsWithoutImprovement, int gridSize);
	
    void run(double mutationRate, bool verbose = false);
    const std::vector<int>& getBestSolution() const;
	int getBestFitness();
private:
    int populationSize;
    double crossoverRate;
    int maxGenerationsWithoutImprovement;
    int gridSize;
    std::vector<std::vector<int>> population;
    std::vector<int> bestSolution;
    int bestFitness;
    std::mt19937 rng;
	
    void initializePopulation();
    void evaluateFitness();
    int countFilledCells(const std::vector<std::vector<int>>& grid) const;
    std::vector<std::vector<int>> decodeGrid(const std::vector<int>& individual) const;
	
    void selection();
    void crossover();
    void mutate(double mutationRate);
    void onePointCrossover(std::vector<int>& parent1, std::vector<int>& parent2);
};

#endif // GENETICALGORITHM_HPP

