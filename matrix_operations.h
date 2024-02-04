#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H
#define CHUNCK_SIZE 8

typedef struct {
    int** A;
    int** BT;
    int** AB;
    int N;
    int K;
    int start;
    int end;
} ThreadData;

int** extract_transpose(const char* fileName, int N, int* numCols);
int** extract_matrix(const char* fileName, int* numRows, int* numCols);
void free_matrix(int** matrix, int rows);
void print_matrix(int** matrix, int M, int N);
int** multiply_matrices(int** A, int rowsA, int colsA, int** B, int rowsB, int colsB);
int** modified_multiply(int** A, int rowsA, int colsA, int** BT, int rowsBT, int colsBT);
int get_threads_amount();
void* process_block(void* arg);

#endif  // MATRIX_OPERATIONS_H
