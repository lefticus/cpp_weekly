#!/bin/bash

# This script will time each executable in a directory 5 times, average the times, and report the average timed run for each executable


# Loop through all executables in the directory
for executable in *; do

  # Skip any files that have an extension
  if [[ "$executable" == *.* ]]; then
    continue
  fi
  
  # Skip any files that do not have executable bit set
  if [[ ! -x "$executable" ]]; then
    continue
  fi

  # Get the file name
  filename=$(basename "$executable")

  # Print the executable name
  echo "Executable: $filename"
  
  # Create a variable to hold the total time
  total_time=0
  total_ram=0

  # Loop five times
  for i in {1..5}; do
    # Time the executable and save the output to a variable
    run_time=$(/usr/bin/time -f "%e %M" "./$executable" 2>&1 1>/dev/null )
    # Extract the time from the output
    time=$(echo "$run_time" | awk '{print $1}')
    ram=$(echo "$run_time" | awk '{print $2}')
    # Add the time to the total
    total_time=$(echo "$total_time + $time" | bc)
    total_ram=$(echo "$total_ram + $ram" | bc)
  done

  # Calculate the average time
  average_time=$(echo "$total_time / 5" | bc -l)
  average_ram=$(echo "$total_ram / 5" | bc -l)

  # Print the average time
  echo "Average time: $average_time"
  echo "Average RAM usage: $average_ram"
done
