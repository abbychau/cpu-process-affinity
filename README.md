# CPU Process Affinity Benchmark

This project provides tools to benchmark and analyze performance characteristics of Intel CPUs with hybrid architecture (P-cores and E-cores) on Windows systems.

## Components

- `main.cpp`: Demonstrates basic P-core and E-core task allocation
- `bench.cpp`: Performs comprehensive benchmarking of core switching and performance

## Usage

### Basic Core Assignment Demo
```cmd
main.exe
```

### Benchmarking Tool
```cmd
bench.exe              # Run mixed P-core and E-core benchmark
bench.exe --p-core-only # Run P-core only benchmark
bench.exe --e-core-only # Run E-core only benchmark
```

## Build

### Using Visual Studio
1. Open the solution file in Visual Studio 2019 or later
2. Select Release configuration
3. Build Solution (F7)

### Using Command Line
Build with MSVC compiler from Developer Command Prompt:

```cmd
cl /O2 main.cpp
cl /O2 bench.cpp
```

## Analysis and Results

For detailed analysis and benchmark results, check out:
[Benchmarking Context Switching Between P-cores and E-cores on Intel CPUs](https://abbychau.github.io/article/benchmarking-context-switching-between-p-cores-and-e-cores-on-intel-cpus)

```
# Benchmark Results on Intel 13700KF
.\bench.exe 
Number of P-Cores: 8
Number of E-Cores: 8
Benchmark completed in 11180 milliseconds.
.\bench.exe --p-core-only
Number of P-Cores: 8
Number of E-Cores: 8
Benchmark completed in 7261 milliseconds.
.\bench.exe --e-core-only
Number of P-Cores: 8
Number of E-Cores: 8
Benchmark completed in 7447 milliseconds.
```

## Requirements

- Windows 10/11 operating system
- CPU with hybrid architecture (e.g., Intel 12th gen or later)
- Visual Studio 2019 or later, or MSVC compiler
