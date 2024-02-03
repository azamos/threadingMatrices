#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    int** A;/*MxN*/
    int** BT;/*KxN*/
    int** AB;/*MxK*/
    int N;
    int K;
    int start;
    int end;
} ThreadData;

/*A is MxN, and B is NxK -> BT is KxN*/

int matrix1[4][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12},
    {13, 14, 15, 16}
};

int matrix2[4][4] = {
    {3, 2, 1, 4},
    {6, 3, 1, 1},
    {1, 1, 0, 0},
    {0, 0, 1, 1}
};

int expected_result[4][4] = {
    {8,11,7,10},
    {58,35,19,34},
    {98,59,31,58},
    {138,83,43,82}
};

int get_threads_amount() {
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores < 1) {
        fprintf(stderr, "Error retrieving the number of available cores.\n");
        return 1;
    }
    printf("Number of available cores: %ld\n", num_cores);
    return num_cores;
}

void* mul_line(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int** A = data->A;;/*MxN*/
    int** BT = data->BT;/*KxN*/
    int** AB = data->AB;/*MxK*/
    int N = data->N;
    int K = data->K;
    int start = data->start;
    int end = data->end;;

    for(int i = start; i< end; i++){/*Go over which lines of AB to produce*/
        /*To produce line i of AB, I mush produce each element j at AB[i]
        AB[i][j] = line A[i] mul line B[j]*/
        for(int j = 0; j< N; j++){
            int sum = 0;
            for(int k = 0; k < K; k++){
                sum += A[i][k]*BT[j][k];
            }
            AB[i][j] = sum;
        }
    }

    pthread_exit(NULL);
}

int main() {
    int n = get_threads_amount();
    printf("%d\n", n);
    pthread_t* threads = (pthread_t*)malloc(n * sizeof(pthread_t));

    // Create a 2D array to store the result of each thread
    int AB[4][4];

    int quota = 4 / n;
    int remainder = 4 % n;
    int start = 0;
    for (int i = 0; i < n; i++) {
        int end = start + quota + (i < remainder ? 1 : 0);

        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->A = matrix1;
        data->BT = matrix2;
        data->AB = AB;
        data->N = 4;
        data->K = 4;
        data->start = start;
        data->end = end;
        pthread_create(&threads[i], NULL, mul_line, (void*)data);

        start = end;
    }

    // Wait for all threads to finish
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the results
    printf("\nResult of line-wise multiplication:\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%d ", AB[i][j]);
        }
        printf("\n");
    }

    // Cleanup
    free(threads);

    return 0;
}
