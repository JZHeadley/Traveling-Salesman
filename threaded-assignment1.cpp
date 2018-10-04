#include <string>
#include <map>
#include <string.h>
#include <limits.h>

#include "include/assignment1.h"

double** distances;
map<long long int, PathCost> solutionsMap;
vector<int> bestPath;
double bestCost;

void tsp(vector<City> cities, int start, int numCities)
{
    long long key = 0x00000;
    vector<int> cityNums;
    // convert cities back to integer array
    for (int i = 1; i < numCities; i++) {
        cityNums.push_back(i);
    }

    // initalize first 2 levels of the lookup table
    for (int i = 1; i < numCities; i++) {
        for (int j = 1; j < numCities; j++) {
            if (i == j)
                continue;
            vector<int> iSet{ i };
            genKey(iSet, j, key);
            PathCost pathCost;
            vector<int> path{ 0, i };
            pathCost.path = path;
            pathCost.cost = distances[i][j] + distances[0][i];
            solutionsMap.insert(pair<long long, PathCost>(key, pathCost));
        }
    }
    //we're good so far...
    double currentCost = 0;

    for (int i = 2; i < numCities; i++) { // iterate through all cardinalities of subsets
        printf("working on subsets of size %i\n", i);
        vector<vector<int> > subsets = generateSubsets(i, cityNums.size());
        for (vector<int> set : subsets) {
            for (int k : set) {
                vector<int> kSet{ k };
                vector<int> diff;
                set_difference(set.begin(), set.end(), kSet.begin(), kSet.end(), inserter(diff, diff.begin()));
                double minCost = INT_MAX;
                vector<int> minPath;
                int bestM;
                // we initialized 2 levels earlier so this for loop will always be able to run.
                for (int m : diff) {
                    vector<int> mSet{ m }; // need to generate the key for k-1
                    vector<int> noMoreM;   // get rid of m because thats where we're going
                    set_difference(diff.begin(), diff.end(), mSet.begin(), mSet.end(), inserter(noMoreM, noMoreM.begin()));

                    genKey(noMoreM, m, key);
                    currentCost = solutionsMap[key].cost + distances[m][k];
                    if (currentCost < minCost) {
                        minCost = currentCost;
                        minPath = solutionsMap[key].path;
                        bestM = m;
                    }
                }
                genKey(diff, k, key);

                PathCost pathCost;
                pathCost.cost = minCost;
                minPath.push_back(bestM);
                pathCost.path = minPath;
                solutionsMap.insert(pair<long long, PathCost>(key, pathCost));
            }
        }
    }
    double minCost = INT_MAX;
    vector<int> minPath;
    int bestM;
    for (int m : cityNums) {
        vector<int> mSet{ m }; // need to generate the key for k-1
        vector<int> noMoreM;   // get rid of m because thats where we're going
        set_difference(cityNums.begin(), cityNums.end(), mSet.begin(), mSet.end(), inserter(noMoreM, noMoreM.begin()));

        genKey(noMoreM, m, key);
        currentCost = solutionsMap[key].cost + distances[m][0];
        if (currentCost < minCost) {
            minCost = currentCost;
            vector<int> path = solutionsMap[key].path;
            minPath = path;
            bestM = m;
        }
    }

    minPath.push_back(bestM);
    minPath.push_back(0);
    bestPath = minPath;
    bestCost = minCost;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("Usage:  ./tsp <dataset path>\n");
        exit(1);
    }
    vector<City> cities = readCities(argv[1]);
    if (cities.size() == 0) {
        printf("Please use a dataset file that is not empty\n");
        exit(2);
    }

    for (City city : cities) {
        cout << "X is " << city.x << " Y is " << city.y << " for city with id " << city.id << endl;
    }

    //distances = computeDistanceMatrix(cities);
    //printMatrix(distances, cities.size(), cities.size());

    int* cityIds = (int*)malloc(cities.size() * sizeof(int));
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    //tsp(cities, 0, cities.size());

    printPath(bestPath);
    printf("cost was %f\n", bestCost);

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t diff = (1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e6;

    printf("TSP ran in %llu ms for %i cities\n", (long long unsigned int)diff, cities.size());
}
