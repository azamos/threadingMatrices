# Matrix Multiplication Performance Comparison

This repository contains code for testing and comparing the performance of different matrix multiplication approaches in C. The experiments include single-threaded standart matrix multiplication(linesXcolumns), a modified single-threaded algorithm(linesXlines of transpose), and a multi-threaded matrix multiplication approach(linesXlines of transpsose).

## Experiment Overview

### Matrix Multiplication Approaches

1. **Naive Single-Threaded Algorithm**: Standard matrix multiplication without optimization.
2. **Modified Single-Threaded Algorithm**: A version of the single-threaded approach optimized by operating on lines of the first matrix and corresponding lines of its transpose.
3. **Multi-Threaded Matrix Multiplication**: Parallel matrix multiplication using pthreads, similar to the modified approach, only multi-threaded.

### Experiment Results

The experiments aim to compare the execution time of these approaches for matrices of varying sizes, specifically focusing on large matrices to assess the effectiveness of multi-threading.

## How to Replicate the Experiment

### Prerequisites
- Run on a linux machine
- Make sure you have a C compiler installed (e.g., GCC).

### Steps

1. Clone the repository:

   ```bash
   git clone https://github.com/azamos/threadingMatrices.git

2. Change directory to the project's folder:

   ```bash
   cd threadingMatrices

3. Compile project:

   ```bash
   gcc main.c matrix_operations.c -o main


4. Either run it without any paramaters:

   ```bash
   ./main

5. OR provide 2 matrices files paths. For example,using the provided matrix1, matrix2 files:

   ```bash
   ./main ./matrix1 ./matrix2