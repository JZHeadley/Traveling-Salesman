#!/bin/bash

for i in {2..16};
do
    numCities=$(echo 2^${i} | bc)
    echo $numCities
    ./genCities.sh ${numCities} > datasets/${numCities}.cities
    output=`./threaded-tsp datasets/${numCities}.cities | tail -n 2`
    echo ${output} | tee -a results.txt
done
