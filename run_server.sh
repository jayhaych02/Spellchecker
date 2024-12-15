#!/bin/bash

# Default values from config
PORT=8888
THREADS=5
BUFFER=10
PRIO=1
DICT="dictAM.txt"

# Parse command line arguments
while getopts "p:w:b:e:d:" opt; do
  case $opt in
    p) PORT="$OPTARG";;
    w) THREADS="$OPTARG";;
    b) BUFFER="$OPTARG";;
    e) PRIO="$OPTARG";;
    d) DICT="$OPTARG";;
    ?) echo "Invalid option -$OPTARG" >&2;;
  esac
done

# Run server with parameters
./server.out -k $PORT -w $THREADS -j $BUFFER -e $PRIO -b $DICT