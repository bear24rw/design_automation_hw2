CXX = g++
CXXFLAGS = -std=c++0x -Wall -lm -g -pg

all:
	$(CXX) $(CXXFLAGS) -o main main.cpp magic.cpp

test:
	$(CXX) $(CXXFLAGS) -o main main.cpp magic.cpp test.cpp -DTEST && ./main

clean:
	rm -fr main
