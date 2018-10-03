#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <string.h>
#include <limits.h>

#include "include/assignment1.h"

using namespace std;

typedef struct {
    double cost;
    vector<int> path;
} PathCost;

double** distances;
map<long long int, PathCost> solutionsMap;
vector<int> bestPath;

double distance(City c1, City c2)
{
    return sqrt(pow(c1.x - c2.x, 2) + pow(c1.y - c2.y, 2));
}

vector<City> readCities(char* filePath)
{
    vector<City> cities;
    ifstream file(filePath);
    double x, y;
    int id = 0;
    // Reading in the cities tried to methodize this and failed
    while (file >> x >> y) {
        City city;
        city.id = id;
        city.x = x;
        city.y = y;
        cities.push_back(city);
        id++;
    }
    printf("We just read in %i cities\n", cities.size());
    return cities;
}

double** computeDistanceMatrix(vector<City> cities)
{
    double** distances = (double**)malloc(cities.size() * sizeof(double*));
    for (int i = 0; i < cities.size(); i++)
        distances[i] = (double*)malloc(cities.size() * sizeof(double));

    for (int i = 0; i < cities.size(); i++) {
        for (int j = 0; j < cities.size(); j++) {
            City city1 = cities[i];
            City city2 = cities[j];
            distances[i][j] = sqrt(pow(city1.x - city2.x, 2) + pow(city1.y - city2.y, 2));
        }
    }
    return distances;
}

long long genKey(vector<int> set, int z)
{
    long long key = 0;
    key |= z;
    for (int j : set) {
        key |= 1 << j + 8;
    }
    return key;
}

vector<vector<int> > generateSubsets(int size, int n)
{
    int count = 0;
    vector<vector<int> > container;
    vector<int> row;
    vector<bool> v((unsigned long)n);
    fill(v.begin(), v.begin() + size, true);

    do {
        for (int i = 0; i < n; ++i) {
            if (v[i]) {
                count++;
                row.push_back(i + 1);
                if (count == size) {
                    container.push_back(row);
                    row.clear();
                    count = 0;
                }
            }
        }
    } while (prev_permutation(v.begin(), v.end()));
    return container;
}

void tsp(vector<City> cities, int start, int numCities)
{
    vector<int> cityNums;
    // convert cities back to integer array
    for (int i = 1; i < numCities; i++) {
        cityNums.push_back(i);
    }

    // initalize first 2 levels of the lookup table
    for (int i = 1; i < numCities; i++) {
        long long key = 0x00000;
        // key |= (long long)(pow(2, i));
        key |= 1 << i + 8;
        for (int j = 1; j < numCities; j++) {
            if (i == j)
                continue;
            key |= j;
            PathCost pathCost;
            printf("key is 0x%x\n", key);
            vector<int> path{ 0, i, j };
            pathCost.path = path;
            pathCost.cost = distances[i][j] + distances[0][i];
            // printf("cost is %f with path: ", pathCost.cost);
            // for (int b = 0; b < path.size(); b++) {
            // printf("%i -> ", path[b]);
            // }
            // printf("\n");
            solutionsMap.insert(pair<long long, PathCost>(key, pathCost));
        }
    }
    //we're good so far...
    long long key = 0;
    double currentCost = 0;

    for (int i = 2; i < numCities; i++) { // cardinality of the set
        vector<vector<int> > subsets = generateSubsets(i, cityNums.size() - 1);
        for (vector<int> set : subsets) {
            for (int k : set) {
                vector<int> kSet{ k };
                vector<int> diff;
                set_difference(set.begin(), set.end(), kSet.begin(), kSet.end(), inserter(diff, diff.begin()));
                int minCost = INT_MAX;
                vector<int> minPath;
                // we initialized 2 levels earlier so this for loop will always be able to run.
                for (int m : diff) {
                    vector<int> mSet{ m }; // need to generate the key for k-1
                    vector<int> noMoreM;   // get rid of m because thats where we're going
                    set_difference(diff.begin(), diff.end(), mSet.begin(), mSet.end(), inserter(noMoreM, noMoreM.begin()));

                    key = genKey(noMoreM, m);
                    currentCost = solutionsMap[key].cost + distances[m][k];
                    if (currentCost < minCost) {
                        minCost = currentCost;
                        vector<int> path = solutionsMap[key].path;
                        path.push_back(m);
                        minPath = path;
                    }
                    // printf("%i ", m);
                }
                key = genKey(diff, k);
                PathCost pathCost;
                pathCost.cost = minCost;
                pathCost.path = minPath;
                printf("path cost is %f with path: ", minCost);
                for (int b = 0; b < minPath.size(); b++) {
                    printf("%i -> ", minPath[b]);
                }
                printf("\n");
            }
        }
    }
    key = genKey(cityNums, 0);
    bestPath = solutionsMap[key].path;
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

    distances = computeDistanceMatrix(cities);
    printMatrix(distances, cities.size(), cities.size());

    int* cityIds = (int*)malloc(cities.size() * sizeof(int));

    tsp(cities, 0, cities.size());

    // printf("Best path is: ");
    // for (int i = 0; i < bestPath.size(); i++) {
    // printf("%i -> ", bestPath[i]);
    // }
    // printf("\n");
}
