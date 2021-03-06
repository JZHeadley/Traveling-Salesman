#include <string>
#include <map>
#include <string.h>
#include <limits.h>
#include <algorithm>
#include <mpi.h>

#include "include/assignment1.h"
#define BLOCK_NUM_TAG 6
#define NUM_BLOCKS_TAG 66
#define BLOCK_CITIES_TAG 666
#define NUM_CITIES_TAG 6666
#define CITIES_RESULT_TAG 66666
#define COST_RESULT_TAG 7
#define NUM_BLOCKS_RECV_TAG 77

int BLOCK_SIZE = 4;
vector<BlockSolution> blockSolutions{};

int numProcs;
int procNum;
vector<City> convPathToCityPath(vector<City> cities, vector<int> positions)
{
    vector<City> truePath{};
    for (int cityNum : positions)
    {
        truePath.push_back(cities[cityNum]);
    }
    return truePath;
}

BlockSolution tsp(vector<City> cities)
{
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
        BlockSolution blockSolution;
        return blockSolution;
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
                    blockSolution.blockId = procNum;

                    return blockSolution;
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
    double tripHome = distance(fullPath[0], fullPath[fullPath.size()]);
    totalCost += tripHome;
    printf("cost of the final trip back home is %.2f\n", tripHome);
    fullPath.push_back(fullPath[0]);

    // returning the final solution
    TSPSolution solution;
    solution.cost = totalCost;
    solution.path = fullPath;
    return solution;
}

int main(int argc, char *argv[])
{
    struct timespec start, end;

    if (argc < 3)
    {
        printf("Usage:  ./tsp <dataset path> <block width>\n");
        exit(1);
    }

    MPI_Init(&argc, &argv);
    const int numItems = 3;
    int blockLengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE};
    MPI_Datatype mpi_city_type;
    MPI_Aint offsets[3];
    offsets[0] = offsetof(City, id);
    offsets[1] = offsetof(City, x);
    offsets[2] = offsetof(City, y);

    MPI_Type_create_struct(numItems, blockLengths, offsets, types, &mpi_city_type);
    MPI_Type_commit(&mpi_city_type);

    MPI_Comm_rank(MPI_COMM_WORLD, &procNum);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    vector<City> cities;
    int numBlocks;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    if (procNum == 0)
    {
        BLOCK_SIZE = atoi(argv[2]);
        cities = readCities(argv[1]);
        if (cities.size() == 0)
        {
            printf("Please use a dataset file that is not empty\n");
            exit(2);
        }

        cities = breakAndSort(cities);
        vector<vector<City>> blockedCities = breakIntoBlocks(cities, BLOCK_SIZE);
        printBlockedCities(blockedCities);

        numBlocks = (int)blockedCities.size();
        for (int i = 1; i < numProcs; i++) // send how many blocks we have to all processes so they know if they're needed
        {
            MPI_Send(&numBlocks, 1, MPI_INT, i, NUM_BLOCKS_TAG, MPI_COMM_WORLD);
        }

        if (numBlocks < numProcs - 1)
        {
            for (int i = 0; i < numBlocks; i++)
            {
                // send number of blocks to be processed to the proccess
                printf("sending block %i to process %i\n", i, i + 1);
                int blocksToSend = 1;
                MPI_Send(&blocksToSend, 1, MPI_INT, i + 1, NUM_BLOCKS_RECV_TAG, MPI_COMM_WORLD);
                int size = blockedCities[i].size();
                MPI_Send(&size, 1, MPI_INT, i + 1, BLOCK_NUM_TAG, MPI_COMM_WORLD);
                City *block = (City *)malloc(size * sizeof(City));
                copy(blockedCities[i].begin(), blockedCities[i].end(), block);
                MPI_Send(block, size, mpi_city_type, i + 1, BLOCK_CITIES_TAG, MPI_COMM_WORLD);
            }
        }
        else // numBlocks >= numProcs - 1
        {
            int blocksLeft = numBlocks;
            int blockToSend = 0;
            for (int i = 0; i < numProcs - 1; i++) // for each worker process
            {
                int blocksToSend = min((int)ceil(numBlocks / (float)(numProcs - 1)), blocksLeft);
                MPI_Send(&blocksToSend, 1, MPI_INT, i + 1, NUM_BLOCKS_RECV_TAG, MPI_COMM_WORLD);
                for (int j = 0; j < blocksToSend; j++) // send min(ceil(n / (float)numProcesses), numProcessesLeft) processes
                {

                    int size = blockedCities[i].size();
                    MPI_Send(&size, 1, MPI_INT, i + 1, BLOCK_NUM_TAG, MPI_COMM_WORLD);
                    City *block = (City *)malloc(size * sizeof(City));
                    copy(blockedCities[blockToSend].begin(), blockedCities[blockToSend].end(), block);
                    MPI_Send(block, size, mpi_city_type, i + 1, BLOCK_CITIES_TAG, MPI_COMM_WORLD);
                    blockToSend++;
                    blocksLeft--;
                }
            }
        }
    }
    else
    {
        MPI_Status status;
        MPI_Recv(&numBlocks, 1, MPI_INT, 0, NUM_BLOCKS_TAG, MPI_COMM_WORLD, &status);
        if (procNum > numBlocks)
        {
            // kill off excess processes if we don't need them since my blocking doesn't split
            // in a way such that we can evenly distribute to all processes
            MPI_Finalize();
            return 0;
        }
        // while we don't have all the blocks we'll do
        int numBlocksToRecv;
        MPI_Recv(&numBlocksToRecv, 1, MPI_INT, 0, NUM_BLOCKS_RECV_TAG, MPI_COMM_WORLD, &status);
        printf("Process %i is still living and is going to receive %i blocks\n", procNum, numBlocksToRecv);

        while (numBlocksToRecv)
        {
            int length;
            MPI_Recv(&length, 1, MPI_INT, 0, BLOCK_NUM_TAG, MPI_COMM_WORLD, &status);
            // printf("The length of the block will be %i\n", length);
            City *block = (City *)malloc(length * sizeof(City));
            MPI_Recv(block, length, mpi_city_type, 0, BLOCK_CITIES_TAG, MPI_COMM_WORLD, &status);
            vector<City> cities;
            for (int i = 0; i < length; i++)
            {
                cities.push_back(block[i]);
            }
            BlockSolution solution = tsp(cities);
            int size = solution.path.size();
            MPI_Send(&size, 1, MPI_INT, 0, NUM_CITIES_TAG, MPI_COMM_WORLD);

            City *cityPath = (City *)calloc(size, sizeof(City));
            copy(solution.path.begin(), solution.path.end(), cityPath);
            MPI_Send(cityPath, size, mpi_city_type, 0, CITIES_RESULT_TAG, MPI_COMM_WORLD);
            printf("Cost of block %i of process %i was %.2f\n", numBlocksToRecv, procNum, solution.cost);
            MPI_Send(&solution.cost, 1, MPI_DOUBLE, 0, COST_RESULT_TAG, MPI_COMM_WORLD);
            numBlocksToRecv--;
        }
    }
    // lets do the actual work

    if (procNum == 0)
    {
        MPI_Status status;
        int numBlocksrecvd = 0;
        vector<BlockSolution> blockSolutions{};
        while (numBlocksrecvd != numBlocks)
        {
            int length;
            MPI_Recv(&length, 1, MPI_INT, MPI_ANY_SOURCE, NUM_CITIES_TAG, MPI_COMM_WORLD, &status);
            City *cityPath = (City *)calloc(length, sizeof(City));
            int source = status.MPI_SOURCE;
            printf("Getting %i cities from node %i\n", length, source);
            MPI_Recv(cityPath, length, mpi_city_type, source, CITIES_RESULT_TAG, MPI_COMM_WORLD, &status);
            vector<City> cities;
            for (int i = 0; i < length; i++)
            {
                cities.push_back(cityPath[i]);
            }
            double pathCost;
            MPI_Recv(&pathCost, 1, MPI_DOUBLE, source, COST_RESULT_TAG, MPI_COMM_WORLD, &status);
            BlockSolution solution;
            solution.cost = pathCost;
            solution.path = cities;
            solution.blockId = source;
            blockSolutions.push_back(solution);
            numBlocksrecvd++;
        }
        TSPSolution solution = stitchBlocks(blockSolutions);
        printf("The final cost is %.2f for this set of %i cities\n", solution.cost, (int)cities.size());
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        uint64_t diff = (1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e6;

        printf("TSP ran in %llu ms for %lu cities\n", (long long unsigned int)diff, cities.size());
    }
    MPI_Finalize();
}
