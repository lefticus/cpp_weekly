#!/bin/bash

if [ $# -lt 2 ]; then
  echo "Usage: $0 <source_file.cpp> <number_of_runs> [additional_compiler_flags]"
  exit 1
fi

# Check if necessary commands are available
commands=("perf" "strip" "/usr/bin/time" "lscpu" "sha256sum")
for cmd in "${commands[@]}"; do
  if ! command -v $cmd &> /dev/null; then
    echo "$cmd could not be found. Please install $cmd and try again."
    exit 1
  fi
done

SOURCE_FILE=$1
NUM_RUNS=$2
ADDITIONAL_FLAGS="${@:3}"
OUTPUT_FILE="performance_stats.csv"

# Compiler and optimization flags
COMPILERS=("/usr/lib/llvm16/bin/clang++"  "clang++" "g++")
OPT_FLAGS=("-O0" "-O1" "-O2" "-O3" "-Os" "-Og")
ARCH_FLAGS=("" "-march=native")
CLANG_STDLIBS=("-stdlib=libc++" "-stdlib=libstdc++")

# Get CPU model information
CPU_MODEL=$(lscpu | grep "Model name" | awk -F: '{print $2}' | xargs)
VIRTUALIZATION=$(lscpu | grep "Virtualization" | awk -F: '{print $2}' | xargs)
VIRT_TYPE=$(lscpu | grep "Hypervisor vendor" | awk -F: '{print $2}' | xargs)
SHA=$(sha256sum "$SOURCE_FILE" | awk '{print $1}')

# CSV header
HEADER="Timestamp,Compiler,Version,Optimization,Architecture,Additional Flags,CPU Model,Virtualization,Virtualization Type,SHA,File Size (bytes),Stripped Size (bytes),Stdlib,Time (ms),RAM Usage (KB),Cycles,Cache References,Cache Misses,Cache Miss Ratio,Instructions,Instructions Per Cycle,Branches,Branch Misses,Branch Miss Ratio"

# Check if the header exists and matches
if [ -f "$OUTPUT_FILE" ]; then
  existing_header=$(head -n 1 "$OUTPUT_FILE")
  if [ "$existing_header" != "$HEADER" ]; then
    echo "$HEADER" > "$OUTPUT_FILE"
  fi
else
  echo "$HEADER" > "$OUTPUT_FILE"
fi

# Function to extract and clean perf metrics
extract_perf_metrics() {
  local perf_output=$1
  local cycles=$(echo "$perf_output" | grep "cycles" -A 1 | head -n 1 | awk '{print $1}' | tr -d ',')
  local cache_references=$(echo "$perf_output" | grep "cache-references" -A 1 | head -n 1 | awk '{print $1}' | tr -d ',')
  local cache_misses=$(echo "$perf_output" | grep "cache-misses" -A 1 | head -n 1 | awk '{print $1}' | tr -d ',')
  local cache_miss_ratio=$(echo "$perf_output" | grep "cache-misses" -A 1 | head -n 1 | awk '{print $4}' | tr -d ',')
  local instructions=$(echo "$perf_output" | grep "instructions" -A 1 | head -n 1 | awk '{print $1}' | tr -d ',')
  local instructions_per_cycle=$(echo "$perf_output" | grep "instructions" -A 1 | head -n 1 | awk '{print $4}' | tr -d ',')
  local branches=$(echo "$perf_output" | grep "branches" -A 1 | head -n 1 | awk '{print $1}' | tr -d ',')
  local branch_misses=$(echo "$perf_output" | grep "branch-misses" -A 1 | head -n 1 | awk '{print $1}' | tr -d ',')
  local branch_miss_ratio=$(echo "$perf_output" | grep "branch-misses" -A 1 | head -n 1 | awk '{print $4}' | tr -d ',')
  
  echo "$cycles,$cache_references,$cache_misses,$cache_miss_ratio,$instructions,$instructions_per_cycle,$branches,$branch_misses,$branch_miss_ratio"
}

# Function to run tests and log results
run_tests() {
  local compiler=$1
  local compiler_version=$2
  local opt=$3
  local arch=$4
  local stdlib=$5

  echo "Compiling with $compiler $opt $arch $stdlib $ADDITIONAL_FLAGS..."
  output_executable="a.out"
  if [[ "$compiler" == "clang++" ]]; then
    $compiler $opt $arch $stdlib $ADDITIONAL_FLAGS $SOURCE_FILE -o $output_executable
  else
    $compiler $opt $arch $ADDITIONAL_FLAGS $SOURCE_FILE -o $output_executable
  fi

  if [ $? -ne 0 ]; then
    echo "Compilation failed for $compiler $opt $arch $stdlib $ADDITIONAL_FLAGS"
    return
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

    perf_output=$(perf stat -e cycles,cache-references,cache-misses,instructions,branches,branch-misses ./$output_executable 2> perf_output.txt)
    cat perf_output.txt  # Debugging output
    metrics=$(extract_perf_metrics "$(cat perf_output.txt)")

    timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    echo "$timestamp,$compiler,$compiler_version,$opt,$arch,\"$ADDITIONAL_FLAGS\",\"$CPU_MODEL\",\"$VIRTUALIZATION\",\"$VIRT_TYPE\",\"$SHA\",$file_size,$stripped_size,$stdlib,$execution_time,$ram_usage,$metrics" >> "$OUTPUT_FILE"
  done
}

# Main loop
for compiler in "${COMPILERS[@]}"; do
  compiler_version=$($compiler --version | head -n 1)
  for opt in "${OPT_FLAGS[@]}"; do
    for arch in "${ARCH_FLAGS[@]}"; do
      if [[ "$compiler" == "clang++" ]]; then
        for stdlib in "${CLANG_STDLIBS[@]}"; do
          run_tests "$compiler" "$compiler_version" "$opt" "$arch" "$stdlib"
        done
      else
        stdlib="libstdc++"
        run_tests "$compiler" "$compiler_version" "$opt" "$arch" "$stdlib"
      fi
    done
  done
done

echo "Performance data collected in $OUTPUT_FILE"
