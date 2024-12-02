CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
SRC = main.cpp GeneticAlgorithm.cpp CellularAutomaton.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = main.out

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(EXEC)

