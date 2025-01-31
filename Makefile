CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: wordrange

wordrange: main.o AVLTree.o
	$(CXX) $(CXXFLAGS) -o wordrange main.o AVLTree.o

main.o: main.cpp AVLTree.h
	$(CXX) $(CXXFLAGS) -c main.cpp

AVLTree.o: AVLTree.cpp AVLTree.h
	$(CXX) $(CXXFLAGS) -c AVLTree.cpp

clean:
	rm -f *.o wordrange

