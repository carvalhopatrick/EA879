#!/bin/bash
cat all_measures.csv | sed 's/-8/-08/' | sed 's/-9/-09/' > temp1
sort -t ',' -k 2 -s temp1 > temp2
sort -t ',' -k 3 -s temp2 > temp3
sort -t ',' -k 1 -s temp3 > temp4
cat temp4 | sed 's/-08/-8/' | sed 's/-09/-9/' > temp5
tr ',' '\t' < temp5 > all_measures_sorted.tab
rm temp1 temp2 temp3 temp4 temp5
