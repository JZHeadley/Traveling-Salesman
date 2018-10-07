#include <string>
#include <map>
#include <string.h>
#include <limits.h>
#include <algorithm>
#include <pthread.h>
#include <mutex>

#include "include/assignment1.h"
int BLOCK_SIZE = 4;
pthread_mutex_t blockMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int blockId;
    vector<City> path;
    double cost;
} BlockSolution;

typedef struct
{
    int threadId;
    vector<City> cities;
} TSPArgs;

vector<BlockSolution> blockSolutions{};

struct sortByX
{
    inline bool operator()(const City city1, const City &city2)
    {
        return (city1.x < city2.x);
    }
};

struct sortByY
{
    inline bool operator()(const City city1, const City &city2)
    {
        return (city1.y < city2.y);
    }
};

vector<vector<vector<City>>> breakIntoMatrixBlocks(vector<City> cities, int blockWidth)
{
    // lets just make a vector out of blockWidth x blockWidth matrices of cities
    // to make life easier
    vector<vector<vector<City>>> blocks{};
    // we only want to run this once loop once for each block we need
    // need this loop so we can address the block we want to add the resulting matrix
    int numElements = (int)cities.size();
    int counter = 0;
    for (int i = 0; i < (cities.size() / ((float)(blockWidth * blockWidth))); i++)
    {
        vector<vector<City>> block{};
        blocks.push_back(block);
        // for each block we need to iterate down 4 or less rows and across 4
        // or less columns which is why we're using the min function
        for (int j = 0; j < min(blockWidth, (int)ceil((numElements - (blockWidth * i + j)) / (float)blockWidth)); j++)
        {

            if (counter == numElements)
                break;
            vector<City> row{};
            blocks[i].push_back(row);
            for (int k = 0; k < blockWidth; k++)
            {
                // couldn't come up with the correct stop condition and wasted way to much time thinking about it and this hack just works sooo....
                // don't worry I'm not happy about it either.  I'll fix it if I have the time but its unlikely I will
                if (counter == numElements)
                    break;
                blocks[i][j].push_back(cities[counter]);
                // printf("counter is %i\n",counter);
                counter++;
            }
        }
    }

    return blocks;
}

vector<City> convPathToCityPath(vector<City> cities, vector<int> positions)
{
    vector<City> truePath{};
    for (int cityNum : positions)
    {
        truePath.push_back(cities[cityNum]);
    }
    return truePath;
}

void *tsp(void *args)
{
    TSPArgs *tspArgs = (TSPArgs *)args;
    int threadId = tspArgs->threadId;
    vector<City> cities = tspArgs->cities;

    double **distances = computeDistanceMatrix(cities);
    map<long long int, PathCost> solutionsMap;
    vector<int> minPath{};
    double minCost = INT_MAX;

    int numCities = (int)cities.size();
    if (numCities == 0)
    {
        vector<int> simple{0};
        minPath = simple;
        minCost = 0;
        return (void *)NULL;
    }
    long long key = 0x00000;
    vector<int> cityNums;
    // convert cities back to integer array
    for (int i = 1; i < numCities; i++)
    {
        cityNums.push_back(i);
    }

    // initalize first 2 levels of the lookup table
    for (int i = 1; i < numCities; i++)
    {
        for (int j = 1; j < numCities; j++)
        {
            if (i == j)
                continue;
            vector<int> iSet{i};
            genKey(iSet, j, key);
            PathCost pathCost;
            vector<int> path{0, i};
            pathCost.path = path;
            pathCost.cost = distances[i][j] + distances[0][i];
            solutionsMap.insert(pair<long long, PathCost>(key, pathCost));
        }
    }
    //we're good so far...
    double currentCost = 0;

    for (int i = 2; i < numCities; i++)
    { // iterate through all cardinalities of subsets
        // printf("working on subsets of size %i\n", i);
        vector<vector<int>> subsets = generateSubsets(i, cityNums.size());
        for (vector<int> set : subsets)
        {
            int count = 0;
            for (int k : set) // for every element of the set lets figure out the ordering
            {
                vector<int> kSet{k};
                vector<int> diff;
                set_difference(set.begin(), set.end(), kSet.begin(), kSet.end(), inserter(diff, diff.begin()));
                double minCost = INT_MAX;
                int bestM;
                // we initialized 2 levels earlier so this for loop will always be able to run.
                for (int m : diff)
                {
                    vector<int> mSet{m}; // need to generate the key for k-1
                    vector<int> noMoreM; // get rid of m because thats where we're going
                    set_difference(diff.begin(), diff.end(), mSet.begin(), mSet.end(), inserter(noMoreM, noMoreM.begin()));

                    genKey(noMoreM, m, key);
                    currentCost = solutionsMap[key].cost + distances[m][k];
                    if (currentCost < minCost)
                    {
                        minCost = currentCost;
                        minPath = solutionsMap[key].path;
                        bestM = m;
                    }
                }
                genKey(diff, k, key);
                count++;

                PathCost pathCost;
                pathCost.cost = minCost;
                minPath.push_back(bestM);
                if (i == numCities - 1 && count == set.size() - 1)
                {
                    // we've finished and found our best path if we get into here.
                    // doing all the finishing logic here because for some reason it doesn't have it
                    // down below outside of all these for loops and I'm not sure why.

                    minPath.push_back(k);
                    // printf("minCost is %f\n", minCost);
                    vector<City> truePath = convPathToCityPath(cities, minPath);
                    BlockSolution blockSolution;
                    blockSolution.path = truePath;
                    blockSolution.cost = minCost;
                    blockSolution.blockId = threadId;

                    pthread_mutex_lock(&blockMutex);

                    blockSolutions.push_back(blockSolution);
                    pthread_mutex_unlock(&blockMutex);

                    break;
                }
                pathCost.path = minPath;
                solutionsMap.insert(pair<long long, PathCost>(key, pathCost));
            }
        }
    }
    free(distances);
    // for some reason minCost isn't what it should be here so I'm going to do my final stuff
    // above where it actually is correct
    // printf("cost was %f\n", minCost);
    // printPath(minPath);
}

vector<City> breakAndSort(vector<City> cities)
{
    vector<City> newCities;
    sort(cities.begin(), cities.end(), sortByX());
    int counter = 0;
    for (int i = 0; i < (cities.size() / (float)BLOCK_SIZE); i++)
    {
        vector<City> tempCities;
        for (int j = 0; j < min((int)(cities.size() - (i * BLOCK_SIZE)), BLOCK_SIZE); j++)
        {
            tempCities.push_back(cities[(i * BLOCK_SIZE) + j]);
            counter++;
        }
        sort(tempCities.begin(), tempCities.end(), sortByY());
        for (int j = 0; j < min((int)(cities.size() - (i * BLOCK_SIZE)), BLOCK_SIZE); j++)
        {
            newCities.push_back(tempCities[j]);
        }
    }
    return newCities;
}

vector<vector<City>> breakIntoBlocks(vector<City> cities, int blockWidth)
{
    vector<vector<City>> blocks{};
    int numElements = (int)cities.size();
    int count = 0;
    for (int i = 0; i < ceil((int)cities.size() / (float)((blockWidth * blockWidth))); i++)
    {
        vector<City> block{};
        blocks.push_back(block);

        for (int j = 0; j < blockWidth * blockWidth; j++)
        {
            if (count == numElements)
                break;
            blocks[i].push_back(cities[i * (blockWidth * blockWidth) + j]);
            count++;
        }
    }
    return blocks;
}

void printBlockedCities(vector<vector<City>> cities)
{
    for (int i = 0; i < (int)cities.size(); i++)
    {
        printf("Block %i\n [", i);
        for (int j = 0; j < (int)cities[i].size(); j++)
        {
            printf("%i:(%.2f, %.2f) ", cities[i][j].id, cities[i][j].x, cities[i][j].y);
        }
        printf("\n\n");
    }
}
typedef struct
{
    double cost;
    vector<City> path;
} TSPSolution;

TSPSolution stitchBlocks(vector<BlockSolution> blockSolutions)
{
    double totalCost = blockSolutions[0].cost;
    vector<City> fullPath = blockSolutions[0].path;
    vector<int> blockPath{blockSolutions[0].blockId};
    vector<BlockSolution> blocksLeft(blockSolutions.begin() + 1, blockSolutions.end());

    while (blocksLeft.size() > 0)
    {
        int bestBlock;
        double bestDist = INT_MAX;
        for (int i = 0; i < (int)blocksLeft.size(); i++)
        {
            double curDist = distance(fullPath[fullPath.size()], blocksLeft[i].path[0]);
            if (curDist < bestDist)
            {
                bestDist = curDist;
                bestBlock = i;
            }
        }
        fullPath.insert(fullPath.end(), blocksLeft[bestBlock].path.begin(), blocksLeft[bestBlock].path.end());
        totalCost += blocksLeft[bestBlock].cost + bestDist;
        blocksLeft.erase(blocksLeft.begin() + bestBlock);
    }
    
    // lets add the path back home
    double tripHome = distance(fullPath[0],fullPath[fullPath.size()]);
    totalCost += tripHome;
    printf("cost of the final trip back home is %.2f\n",tripHome);
    fullPath.push_back(fullPath[0]);

    // returning the final solution
    TSPSolution solution;
    solution.cost = totalCost;
    solution.path = fullPath;
    return solution;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:  ./tsp <dataset path> <block width>\n");
        exit(1);
    }
    vector<City> cities = readCities(argv[1]);
    BLOCK_SIZE = atoi(argv[2]);
    if (cities.size() == 0)
    {
        printf("Please use a dataset file that is not empty\n");
        exit(2);
    }
    pthread_mutex_init(&blockMutex, NULL);

    cities = breakAndSort(cities);
    vector<vector<vector<City>>> blockedMatrixCities = breakIntoMatrixBlocks(cities, BLOCK_SIZE);
    vector<vector<City>> blockedCities = breakIntoBlocks(cities, BLOCK_SIZE);
    printBlockedCities(blockedCities);

    // lets do the actual work
    pthread_t *threads = (pthread_t *)malloc(blockedCities.size() * sizeof(pthread_t));
    int *threadIds = (int *)malloc(blockedCities.size() * sizeof(int));

    for (int i = 0; i < (int)blockedCities.size(); i++)
        threadIds[i] = i;

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    for (int i = 0; i < (int)blockedCities.size(); i++)
    {
        TSPArgs *args = new TSPArgs;
        args->threadId = threadIds[i];
        args->cities = blockedCities[i];
        printf("Spawning thread for block %i\n", i);
        int status = pthread_create(&threads[i], NULL, tsp, (void *)args);
    }

    for (int i = 0; i < blockedCities.size(); i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&blockMutex);

    TSPSolution solution = stitchBlocks(blockSolutions);
    printf("The final cost is %.2f for this set of %i cities\n", solution.cost, (int)cities.size());
    vector<int> bestPathIds{};
    for (City city : solution.path)
    {
        bestPathIds.push_back(city.id);
    }
    printPath(bestPathIds);

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t diff = (1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e6;

    printf("TSP ran in %llu ms for %lu cities\n", (long long unsigned int)diff, (int)cities.size());
}
