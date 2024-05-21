#!/bin/bash

if [ $# -lt 1 ]; then
  echo "Usage: $0 <source_file.cpp> [additional_compiler_flags]"
  exit 1
fi

# Check if necessary commands are available
commands=("perf" "strip" "/usr/bin/time" "lscpu")
for cmd in "${commands[@]}"; do
  if ! command -v $cmd &> /dev/null; then
    echo "$cmd could not be found. Please install $cmd and try again."
    exit 1
  fi
done

SOURCE_FILE=$1
ADDITIONAL_FLAGS="${@:2}"
OUTPUT_FILE="performance_stats.csv"
NUM_RUNS=5

# Compiler and optimization flags
COMPILERS=("g++" "clang++")
OPT_FLAGS=("-O0" "-O1" "-O2" "-O3" "-Os")
ARCH_FLAGS=("-march=x86-64" "-march=x86-64-v2" "-march=x86-64-v3" "-march=x86-64-v4")

# Get CPU model information
CPU_MODEL=$(lscpu | grep "Model name" | awk -F: '{print $2}' | xargs)
VIRTUALIZATION=$(lscpu | grep "Virtualization" | awk -F: '{print $2}' | xargs)
VIRT_TYPE=$(lscpu | grep "Hypervisor vendor" | awk -F: '{print $2}' | xargs)

# CSV header
echo "Timestamp,Compiler,Version,Optimization,Architecture,Additional Flags,CPU Model,Virtualization,Virtualization Type,File Size (bytes),Stripped Size (bytes),Run,Time (ms),RAM Usage (KB),Cache References,Cache Misses,Cache Miss Ratio,Instructions,Instructions Per Cycle,Branches,Branch Misses,Branch Miss Ratio" > $OUTPUT_FILE

# Function to extract and clean perf metrics
extract_perf_metrics() {
  local perf_output=$1
  local cache_references=$(echo "$perf_output" | grep "cache-references" -A 1 | tail -n 1 | awk '{print $1}' | tr -d ',')
  local cache_misses=$(echo "$perf_output" | grep "cache-misses" -A 1 | tail -n 1 | awk '{print $1}' | tr -d ',')
  local cache_miss_ratio=$(echo "$perf_output" | grep "cache-misses" -A 1 | tail -n 1 | awk '{print $4}' | tr -d ',')
  local instructions=$(echo "$perf_output" | grep "instructions" -A 1 | tail -n 1 | awk '{print $1}' | tr -d ',')
  local instructions_per_cycle=$(echo "$perf_output" | grep "instructions" -A 1 | tail -n 1 | awk '{print $4}' | tr -d ',')
  local branches=$(echo "$perf_output" | grep "branches" -A 1 | tail -n 1 | awk '{print $1}' | tr -d ',')
  local branch_misses=$(echo "$perf_output" | grep "branch-misses" -A 1 | tail -n 1 | awk '{print $1}' | tr -d ',')
  local branch_miss_ratio=$(echo "$perf_output" | grep "branch-misses" -A 1 | tail -n 1 | awk '{print $4}' | tr -d ',')
  
  echo "$cache_references,$cache_misses,$cache_miss_ratio,$instructions,$instructions_per_cycle,$branches,$branch_misses,$branch_miss_ratio"
}

# Main loop
for compiler in "${COMPILERS[@]}"; do
  compiler_version=$($compiler --version | head -n 1)
  for opt in "${OPT_FLAGS[@]}"; do
    for arch in "${ARCH_FLAGS[@]}"; do
      echo "Compiling with $compiler $opt $arch $ADDITIONAL_FLAGS..."
      output_executable="a.out"
      $compiler $opt $arch $ADDITIONAL_FLAGS $SOURCE_FILE -o $output_executable

      if [ $? -ne 0 ]; then
        echo "Compilation failed for $compiler $opt $arch $ADDITIONAL_FLAGS"
        continue
      fi

      file_size=$(stat -c%s "$output_executable")
      strip "$output_executable"
      stripped_size=$(stat -c%s "$output_executable")

      for i in $(seq 1 $NUM_RUNS); do
        echo "Running execution $i..."
        start_time=$(date +%s%N)
        /usr/bin/time -v ./a.out 2> time_output.txt
        end_time=$(date +%s%N)

        execution_time=$((($end_time - $start_time) / 1000000))  # in milliseconds
        ram_usage=$(grep "Maximum resident set size" time_output.txt | awk '{print $6}')

        perf_output=$(perf stat -e cache-references,cache-misses,instructions,branches,branch-misses ./$output_executable 2> perf_output.txt)
        cat perf_output.txt  # Debugging output
        metrics=$(extract_perf_metrics "$(cat perf_output.txt)")

        timestamp=$(date +"%Y-%m-%d %H:%M:%S")
        echo "$timestamp,$compiler,$compiler_version,$opt,$arch,\"$ADDITIONAL_FLAGS\",\"$CPU_MODEL\",\"$VIRTUALIZATION\",\"$VIRT_TYPE\",$file_size,$stripped_size,$i,$execution_time,$ram_usage,$metrics" >> $OUTPUT_FILE
      done
    done
  done
done

echo "Performance data collected in $OUTPUT_FILE"
