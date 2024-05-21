#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Usage: $0 <source_file.cpp>"
  exit 1
fi

# Check if necessary commands are available
commands=("perf" "strip" "/usr/bin/time")
for cmd in "${commands[@]}"; do
  if ! command -v $cmd &> /dev/null; then
    echo "$cmd could not be found. Please install $cmd and try again."
    exit 1
  fi
done

SOURCE_FILE=$1
OUTPUT_FILE="performance_stats.csv"

# Compiler and optimization flags
COMPILERS=("gcc" "clang" "clang-13")
OPT_FLAGS=("-O0" "-O1" "-O2" "-O3" "-Os")
ARCH_FLAGS=("-march=x86-64" "-march=x86-64-v2" "-march=x86-64-v3" "-march=x86-64-v4")

# CSV header
echo "Compiler,Version,Optimization,Architecture,File Size (bytes),Stripped Size (bytes),Time1 (ms),Time2 (ms),Time3 (ms),Time4 (ms),Time5 (ms),RAM Usage1 (KB),RAM Usage2 (KB),RAM Usage3 (KB),RAM Usage4 (KB),RAM Usage5 (KB),Cache References,Cache Misses,Cache Miss Ratio,Instructions,Instructions Per Cycle,Branches,Branch Misses,Branch Miss Ratio" > $OUTPUT_FILE

# Function to extract perf metrics
extract_perf_metrics() {
  local perf_output=$1
  local cache_references=$(echo "$perf_output" | grep "cache-references" -A 1 | tail -n 1 | awk '{print $1}')
  local cache_misses=$(echo "$perf_output" | grep "cache-misses" -A 1 | tail -n 1 | awk '{print $1}')
  local cache_miss_ratio=$(echo "$perf_output" | grep "cache-misses" -A 1 | tail -n 1 | awk '{print $4}')
  local instructions=$(echo "$perf_output" | grep "instructions" -A 1 | tail -n 1 | awk '{print $1}')
  local instructions_per_cycle=$(echo "$perf_output" | grep "instructions" -A 1 | tail -n 1 | awk '{print $4}')
  local branches=$(echo "$perf_output" | grep "branches" -A 1 | tail -n 1 | awk '{print $1}')
  local branch_misses=$(echo "$perf_output" | grep "branch-misses" -A 1 | tail -n 1 | awk '{print $1}')
  local branch_miss_ratio=$(echo "$perf_output" | grep "branch-misses" -A 1 | tail -n 1 | awk '{print $4}')
  
  echo "$cache_references,$cache_misses,$cache_miss_ratio,$instructions,$instructions_per_cycle,$branches,$branch_misses,$branch_miss_ratio"
}

# Main loop
for compiler in "${COMPILERS[@]}"; do
  compiler_version=$($compiler --version | head -n 1)
  for opt in "${OPT_FLAGS[@]}"; do
    for arch in "${ARCH_FLAGS[@]}"; do
      echo "Compiling with $compiler $opt $arch..."
      output_executable="a.out"
      $compiler $opt $arch $SOURCE_FILE -o $output_executable

      if [ $? -ne 0 ]; then
        echo "Compilation failed for $compiler $opt $arch"
        continue
      fi

      file_size=$(stat -c%s "$output_executable")
      strip "$output_executable"
      stripped_size=$(stat -c%s "$output_executable")

      times=()
      ram_usages=()
      for i in {1..5}; do
        echo "Running execution $i..."
        start_time=$(date +%s%N)
        /usr/bin/time -v ./a.out 2> time_output.txt
        end_time=$(date +%s%N)

        execution_time=$((($end_time - $start_time) / 1000000))  # in milliseconds
        ram_usage=$(grep "Maximum resident set size" time_output.txt | awk '{print $6}')

        times+=($execution_time)
        ram_usages+=($ram_usage)
      done

      perf_output=$(perf stat -e cache-references,cache-misses,instructions,branches,branch-misses ./$output_executable 2>&1)
      metrics=$(extract_perf_metrics "$perf_output")

      echo "$compiler,$compiler_version,$opt,$arch,$file_size,$stripped_size,${times[0]},${times[1]},${times[2]},${times[3]},${times[4]},${ram_usages[0]},${ram_usages[1]},${ram_usages[2]},${ram_usages[3]},${ram_usages[4]},$metrics" >> $OUTPUT_FILE
    done
  done
done

echo "Performance data collected in $OUTPUT_FILE"

