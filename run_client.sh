#!/bin/bash

# Default values
PORT=8888
IP="127.0.0.1"
MODE="single"  # single or multi

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
    # Run multiple clients in background
    ./client.out &
    ./client.out &
    ./client.out &
    ./client.out &
    ./client.out &
    ;;
  *)
    echo "Invalid mode. Use 'single' or 'multi'"
    exit 1
    ;;
esac