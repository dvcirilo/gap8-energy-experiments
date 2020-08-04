#!/bin/bash
echo "Writing in" $1 "and" $2
./script/energy-measurement.py > $1 & 
make run > $2 && pkill -f '.*energy-measurement.*' &
