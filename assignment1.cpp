#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

#include "include/assignment1.h"

using namespace std;
double** distances;

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

vector<vector<int> > generateSubsets(vector<int> cities, int size)
{
    vector<vector<int> > subsets;

    return subsets;
}

void tsp(vector<City> cities, int start, int numCities)
{
    vector<int> cityNums;
    // convert cities back to integer array
    for (int i = 1; i < numCities; i++) {
        cityNums.push_back(i);
    }
    for (int i = 1; i < numCities; i++) {
        for (vector<int> set : generateSubsets(cityNums, i)) {
            for (int k : set) {
                vector<int> kSet;
                kSet.push_back(k);
                vector<int> diff;
                set_difference(set.begin(), set.end(), kSet.begin(), kSet.end(), inserter(diff, diff.begin()));
                for (int num : diff) {
                    printf("%i ", num);
                }
                printf("\n");
            }
        }
    }
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
}
