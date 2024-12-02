#ifndef CELLULAR_AUTOMATON_HPP
#define CELLULAR_AUTOMATON_HPP

#include <vector>

class CellularAutomaton {
    int rows;
    int cols;
    std::vector<std::vector<int>> state;

public:
	CellularAutomaton(const std::vector<std::vector<int>>& predefinedGrid);
    CellularAutomaton(int rows, int cols, const std::vector<int>& initialState);

    void evolve(int generations);
    const std::vector<std::vector<int>>& getCurrentState() const;

private:
    int countAliveNeighbors(int row, int col) const;
};

#endif // CELLULAR_AUTOMATON_HPP

