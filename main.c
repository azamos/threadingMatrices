#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "matrix_operations.h"
#define BILLION 1000000000L
#define BIG_M 1500
#define BIG_A_PATH "./big_A"
#define BIG_B_PATH "./big_B"

int file_exists(const char *filename) {
    return access(filename, F_OK) != -1;
}

void generate_big_matrices(){
    if (file_exists(BIG_A_PATH) && file_exists(BIG_B_PATH)) {
        printf("\nFiles %s and %s already exist. Skipping generation.\n", BIG_A_PATH, BIG_B_PATH);
        return;
    }
    srand(time(NULL));
    FILE* file_A = fopen(BIG_A_PATH,"w");
    FILE* file_B = fopen(BIG_B_PATH,"w");
    for(int i = 0; i< BIG_M; i++){
        for(int j = 0; j< BIG_M; j++){
            fprintf(file_A,"%d",rand());
            fprintf(file_B,"%d",rand());
            if( j < BIG_M -1 ){
                fprintf(file_A," ");
                fprintf(file_B," ");
            }
        }
        if(i<BIG_M-1){
            fprintf(file_A,"\n");
            fprintf(file_B,"\n");
        }
    }
    fclose(file_A);
    fclose(file_B);

    printf("\nGenerated files %s and %s.\n", BIG_A_PATH, BIG_B_PATH);
}

int main(int argc, char* argv[]){
    char* path1;
    char* path2;
    if(argc<3){
        printf("\nMissing source file for matrix 1 and source file for matrix 2.\nWill generate 2 large matrix files instead...\n");
        generate_big_matrices();
        path1 = BIG_A_PATH;
        path2 = BIG_B_PATH;
    }
    else{
        path1 = argv[1];
        path2 = argv[2];
    }
    int rows1,cols1,rows2,cols2;
    int** matrix1 = extract_matrix(path1,&rows1,&cols1);
    int** matrix2 = extract_matrix(path2,&rows2,&cols2);
    
    // printf("\nmatrix1 has %d rows and %d cols\n",rows1,cols1);
    // print_matrix(matrix1,rows1,cols1);
    // printf("\nmatrix2 has %d rows and %d cols\n",rows2,cols2);
    // print_matrix(matrix2,rows2,cols2);

    if(cols1!=rows2){
        printf("\nError, cannot mutiply matrices due to a dimenstions mismatch\n");
        printf("cols1 = %d, rows2 = %d\n",cols1,rows2);
        free_matrix(matrix1,rows1);
        free_matrix(matrix2,rows2);
        exit(-1);
    }
    //printf("\nmatrix1 and matrix 2 can be mutliplied!\n");
    /*First, I will do multiplication WITHOUT multi-threading, and time it*/
    clock_t start,end;
    double cpu_time_used;
    start = clock();
    int** AB = multiply_matrices(matrix1,rows1,cols1,matrix2,rows2,cols2);
    end = clock();
    cpu_time_used = (((double)(end-start))/CLOCKS_PER_SEC);
    //print_matrix(AB,rows1,cols2);

    printf("\nCPU time used for singlethreaded matrix mul: %f  seconds\n",cpu_time_used);
    printf("\nNow, let's try to transpose and see if it improves time:\n");
    int K;
    int** BT = extract_transpose(path2,cols1,&K);
    // for(int i =0;i<K; i++){
    //     printf("\n");
    //     for(int j = 0; j<cols1;j++){
    //         printf("%d ",BT[i][j]);
    //     }
    // }

    /*Generated transpose from file. Now, to try modified matrix mult*/
    clock_t start2,end2;
    double cpu_time_used2;
    start2 = clock();
    int** AB2 = modified_multiply(matrix1,rows1,cols1,BT,K,cols1);
    end2 = clock();
    cpu_time_used2 = (((double)(end2-start2))/CLOCKS_PER_SEC);
    /*Checking correctness...*/
    short correct = 1;
    for(int i =0; i< rows1;i++){
        for(int j =0; j<K; j++ ){
            if(AB[i][j]!=AB2[i][j]){
                correct = 0;
                break;
            }
        }
    }
    printf("the modified algo is %s ", correct ? "corret":"wrong");
    printf("\noriginal CPU time: %f  seconds,\n modified: %f  seconds\n",cpu_time_used,cpu_time_used2);
    if(cpu_time_used < cpu_time_used2){
        printf("\nThe modified version was not faster\n");
    }
    int cores;
    printf("\nNow to try a %d threaded matrix multiplication\n",cores=get_threads_amount());

    pthread_t* threads = (pthread_t*)malloc(cores*sizeof(pthread_t));
    
    int work_qouta = cols1 / cores;
    printf("work qouta = %d  ",work_qouta);
    int remaining = cols1 % cores;
    ThreadData** datas = (ThreadData**)malloc(cores*sizeof(ThreadData*));
    int** AB3 = (int**)malloc(rows1*sizeof(int*));
    for(int i =0; i <  rows1; i++){
        AB3[i] = (int*)malloc(cols2*sizeof(int));
    }
    struct timespec start3, end3;
    double cpu_time_used3;
    clock_gettime(CLOCK_MONOTONIC, &start3);
    for(int i =0; i < cores; i++){
        int block_start = i*work_qouta;
        // printf("start = %d  ",block_start);
        int block_end = block_start + work_qouta;
        // printf("block end  = %d  ",block_end);
        if(i==cores-1){
            block_end+=remaining;
        }
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        data->A=matrix1;
        data->BT=BT;
        data->AB = AB3;
        data->N = cols1;
        data->K = cols2;
        data->start = block_start;
        data->end = block_end;
        pthread_create(&threads[i],NULL,process_block,(void*)data);
        datas[i] = data;
    }


    for (int i = 0; i < cores; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end3);
    cpu_time_used3 = (end3.tv_sec - start3.tv_sec) + (end3.tv_nsec - start3.tv_nsec) / BILLION;


    short wrong = 0;
    for(int i =0; i< rows1;i++){
        // printf("\n\n");
        for(int j =0; j<K; j++ ){
            // printf("AB[%d][%d] = %d, vs AB3[%d][%d] = %d",i,j,AB[i][j],i,j,AB3[i][j]);
            // printf("\n");
            if(AB[i][j]!=AB3[i][j]){
                wrong = 1;
                break;
            }
        }
    }
    printf("the threaded algo is %s ", wrong == 0? "corret":"wrong");
    printf("\n%d-threaded, modified matrix mul took %f seconds.\n",cores,cpu_time_used3);
    printf("\nIN CONCLUSION: naive algo took %f,\nModified single thread took %f, and modified multi-thread took %f\n",cpu_time_used,cpu_time_used2,cpu_time_used3);
    
    free_matrix(matrix1,rows1);
    free_matrix(matrix2,rows2);
    free_matrix(AB,rows1);
    free_matrix(BT,cols1);
    free_matrix(AB2,cols1);
    free_matrix(AB3,cols1);

    free(threads);
    for(int i = 0; i < cores; i++){
        free(datas[i]);
    }
    free(datas);
    return 0;
}