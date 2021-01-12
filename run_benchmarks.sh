#!/bin/bash
./run.sh clang $1 -O1 NOVERIFY

for ((i=$3;i <=$4 ;i=i+1000))
do
	python run_one.py $i $2
done
./plot.sh $2
exit 0



