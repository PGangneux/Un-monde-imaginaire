# Compilateur OpenMPI pour C++
CXX = mpicxx
CXXFLAGS = -Wall -Wextra -std=c++11 -g -fsanitize=address

TARGET = simulation
SRC = main.c++ monde.c++
OBJ = main.o monde.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c++ monde.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Utilisation : make run NP=5 ROOT=1 (ou make run par défaut 4 processus, root 0)
NP ?= 4
ROOT ?= 0

run: $(TARGET)
	mpirun -np $(NP) ./$(TARGET) $(ROOT)

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all

.PHONY: all run clean rebuild