#ifndef CELLULAR_AUTOMATON_HPP
#define CELLULAR_AUTOMATON_HPP

#include <vector>
#include <string>

class CellularAutomaton {
    const int size = 50; // Размер поля
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> nextGrid;

public:
    CellularAutomaton(const std::vector<std::vector<int>>& initialState);

    void step();
    const std::vector<std::vector<int>>& getGrid() const;
    void saveToFile(const std::string& filename) const;

private:
    int countLiveNeighbors(int x, int y) const;
};

#endif // CELLULAR_AUTOMATON_HPP

