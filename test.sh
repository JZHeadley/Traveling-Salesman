#!/bin/bash

for i in {5..25};
do
    ./genCities.sh ${i} > datasets/${i}.cities
    output=`./tsp datasets/${i}.cities | tail -n 2`
    echo ${output} | tee -a results.txt
done
