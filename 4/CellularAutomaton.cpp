#include "CellularAutomaton.hpp"
#include <fstream>

CellularAutomaton::CellularAutomaton(const std::vector<std::vector<int>>& initialState)
    : grid(initialState), nextGrid(size, std::vector<int>(size, 0)) {}

void CellularAutomaton::step() {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int liveNeighbors = countLiveNeighbors(i, j);
            if (grid[i][j] == 1) {
                nextGrid[i][j] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0;
            } else {
                nextGrid[i][j] = (liveNeighbors == 3) ? 1 : 0;
            }
        }
    }
    grid = nextGrid;
}

const std::vector<std::vector<int>>& CellularAutomaton::getGrid() const {
    return grid;
}

void CellularAutomaton::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    for (const auto& row : grid) {
        for (int cell : row) {
            file << (cell ? "X" : "-");
        }
        file << "\n";
    }
}

int CellularAutomaton::countLiveNeighbors(int x, int y) const {
    static const int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static const int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int count = 0;

    for (int k = 0; k < 8; ++k) {
        int nx = x + dx[k], ny = y + dy[k];
        if (nx >= 0 && ny >= 0 && nx < size && ny < size) {
            count += grid[nx][ny];
        }
    }
    return count;
}

