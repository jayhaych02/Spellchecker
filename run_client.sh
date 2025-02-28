#!/bin/bash

# Default values
PORT=8888
IP="127.0.0.1"
MODE="multi"  # single or multi. type single to run single

# Parse command line arguments
while getopts "p:i:m:" opt; do
  case $opt in
    p) PORT="$OPTARG";;
    i) IP="$OPTARG";;
    m) MODE="$OPTARG";;
    ?) echo "Usage: ./run_client.sh [-p port] [-i ip] [-m mode]" >&2;;
  esac
done

# Run client based on mode
case $MODE in
  "single")
    ./client.out  # Single client
    ;;
  "multi")
    # Run 100 clients in background
    for i in {1..100}
    do
      ./client.out &
      echo "Started client $i"
    done
    ;;
  *)
    echo "Invalid mode. Use 'single' or 'multi'"
    exit 1
    ;;
esac

# Wait for all background processes to finish
wait
echo "All clients completed"