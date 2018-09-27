#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <list>
#include <iostream>
#include <math.h>

#include "include/assignment1.h"

using namespace std;
double distance(City c1, City c2)
{
    return sqrt(pow(c1.x - c2.x, 2) + pow(c1.y - c2.y, 2));
}

int main(int argc, char** argv)
{
    std::list<City> cities;
    std::ifstream file("datasets/5.cities");
    double x, y;
    // Reading in the cities tried to methodize this and failed
    while (file >> x >> y) {
        City city;
        city.x = x;
        city.y = y;
        cities.push_back(city);
    }
    printf("We just read in %i cities\n", cities.size());
    for (City city : cities) {
        cout << "X is " << city.x << " Y is " << city.y << "\n" << endl;
    }
}
