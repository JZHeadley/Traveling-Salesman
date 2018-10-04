

all: tsp threaded-tsp


tsp: assignment1.cpp include/*
	g++ -Wall -g -std=c++11 $^ -o $@

threaded-tsp: threaded-assignment1.cpp include/*
	g++ -Wall -g -std=c++11 $^ -o $@

clean:
	rm -f tsp threaded-tsp

run: all
	./threaded-tsp datasets/16.cities
