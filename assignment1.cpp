#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

#include "include/assignment1.h"

using namespace std;

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

void tsp(double** distances, int start, int numCities)
{
    int secondToLastCity = -1;
    for (int i = 0; i < numCities; i++) {
        if (i == start)
            continue;

        if (secondToLastCity == -1)
            secondToLastCity = i;
        if (distances[i][start] < distances[secondToLastCity][start])
            secondToLastCity = i;
    }
    printf("City %i is the second to last city\n", secondToLastCity);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("Usage:  ./tsp <dataset path>\n");
        exit(1);
    }
    vector<City> cities = readCities(argv[1]);

    for (City city : cities) {
        cout << "X is " << city.x << " Y is " << city.y << " for city with id " << city.id << endl;
    }

    double** distances = computeDistanceMatrix(cities);
    printMatrix(distances, cities.size(), cities.size());

    tsp(distances, 0, cities.size());
}
