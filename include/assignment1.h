#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <iterator>
#include <vector>

using namespace std;

typedef struct {
    int id;
    double x;
    double y;
} City;

typedef struct {
    double cost;
    vector<int> path;
} PathCost;

void printMatrix(double** matrix, int r, int c)
{
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            // printf("distance between city i%i and j%i is %f\n", i, j, matrix[i][j]);
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void printMatrixArray(vector<City> matrix, int rowWidth, int numElements)
{
    int counter = 0;
    for (int i = 0; i < (numElements / (float)rowWidth); i++)
    {
        printf("[ ");
        for (int j = 0; j < min((numElements - (i * rowWidth)), rowWidth); j++)
        {
            City city = matrix[counter];
            printf("(%f, %f) ", city.x, city.y);
            counter++;
        }
        printf("]\n");
    }
}

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
    printf("We just read in %lu cities\n", cities.size());
    return cities;
}

void genKey(vector<int> set, int z, long long& key)
{
    key = 0;
    key |= z;
    for (int j : set) {
        key |= (1 << (j + 8));
    }
}

vector<vector<int>> generateSubsets(int size, int n)
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

double** computeDistanceMatrix(vector<City> cities)
{
    double** distances = (double**)malloc((int)cities.size() * sizeof(double*));
    for (int i = 0; i < (int)cities.size(); i++)
        distances[i] = (double*)malloc((int)cities.size() * sizeof(double));

    for (int i = 0; i < (int)cities.size(); i++) {
        for (int j = 0; j < (int)cities.size(); j++) {
            City city1 = cities[i];
            City city2 = cities[j];
            distances[i][j] = sqrt(pow(city1.x - city2.x, 2) + pow(city1.y - city2.y, 2));
        }
    }
    return distances;
}

void printPath(vector<int> path)
{

    printf("path is: ");
    for (int i = 0; i < (int)path.size() - 1; i++) {
        printf("%i -> ", path[i]);
    }
    printf("%i", path[(int)path.size() - 1]);

    printf("\n");
}
