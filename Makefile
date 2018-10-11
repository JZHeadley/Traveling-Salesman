

all: tsp threaded-tsp mpi-tsp


tsp: assignment1.cpp include/*
	g++ -Wall -g -std=c++11 $^ -o $@

threaded-tsp: threaded-assignment1.cpp include/*
	g++ -Wall -pthread -g -std=c++11 $^ -o $@

mpi-tsp: mpi-assignment1.cpp include/*
	mpic++ -Wall -g -std=c++11 $^ -o $@

clean:
	rm -f tsp threaded-tsp mpi-tsp

run: all
	mpirun -np 6 mpi-tsp datasets/26.cities 4