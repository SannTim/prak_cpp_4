#include "CellularAutomaton.hpp"

CellularAutomaton::CellularAutomaton(int rows, int cols, const std::vector<int>& initialState)
    : rows(rows), cols(cols), state(rows, std::vector<int>(cols, 0)) {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            state[r][c] = initialState[r * cols + c];
        }
    }
}

CellularAutomaton::CellularAutomaton(const std::vector<std::vector<int>>& predefinedGrid)
    : rows(predefinedGrid.size()), 
      cols(predefinedGrid.empty() ? 0 : predefinedGrid[0].size()), 
      state(predefinedGrid) {}

void CellularAutomaton::evolve(int generations) {
    for (int gen = 0; gen < generations; ++gen) {
        std::vector<std::vector<int>> newState = state;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int aliveNeighbors = countAliveNeighbors(r, c);
                if (state[r][c] == 1) {
                    newState[r][c] = (aliveNeighbors == 2 || aliveNeighbors == 3) ? 1 : 0;
                } else {
                    newState[r][c] = (aliveNeighbors == 3) ? 1 : 0;
                }
            }
        }
        state = newState;
    }
}

const std::vector<std::vector<int>>& CellularAutomaton::getCurrentState() const {
    return state;
}

int CellularAutomaton::countAliveNeighbors(int row, int col) const {
    int count = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                count += state[nr][nc];
            }
        }
    }
    return count;
}

