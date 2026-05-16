CXXFLAGS = -std=c++17 -O2 -Wall
LDFLAGS = -lgtest -pthread

all: test

test:
	g++ $(CXXFLAGS) test.cpp interpreter.cpp $(LDFLAGS) -o test

bench:
	g++ $(CXXFLAGS) bench.cpp interpreter.cpp -o bench

.PHONY: all test bench
