

all: tsp


tsp: assignment1.cpp include/*
	g++ -g -std=c++11 $^ -o $@

clean:
	rm -f tsp
