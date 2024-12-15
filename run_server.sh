#!/bin/bash

# Default values
PORT=8888
THREADS=5
BUFFER=10
PRIO=1
DICT="dictAM.txt"
SCHED=2  # Default FIFO scheduling = 1. prio = 2

# Parse command line arguments
while getopts "p:w:b:e:d:s:" opt; do
  case $opt in
    p) PORT="$OPTARG";;
    w) THREADS="$OPTARG";;
    b) BUFFER="$OPTARG";;
    e) PRIO="$OPTARG";;
    d) DICT="$OPTARG";;
    s) SCHED="$OPTARG";;
    ?) echo "Usage: ./run_server.sh [-p port] [-w threads] [-b buffer] [-e prio] [-d dict] [-s sched]" >&2;;
  esac
done

# Run configurations
case $SCHED in
  1)
    echo "Running with FIFO scheduling"
    ;;
  2)
    echo "Running with Priority scheduling"
    ;;
  *)
    echo "Invalid scheduling type. Using default (FIFO)"
    SCHED=1
    ;;
esac

# Run server with parameters
./server.out -k $PORT -w $THREADS -j $BUFFER -e $SCHED -b $DICT