#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include "matrix_operations.h"
#define BILLION 1000000000L


int main(int argc, char* argv[]){
    if(argc<3){
        printf("\nMissing source file for matrix 1 and source file for matrix 2");
        exit(-1);
    }
    int rows1,cols1,rows2,cols2;
    int** matrix1 = extract_matrix(argv[1],&rows1,&cols1);
    int** matrix2 = extract_matrix(argv[2],&rows2,&cols2);
    
    //printf("\nmatrix1 has %d rows and %d cols\n",rows1,cols1);
    //print_matrix(matrix1,rows1,cols1);
    //printf("\nmatrix2 has %d rows and %d cols\n",rows2,cols2);
    //print_matrix(matrix2,rows2,cols2);

    if(cols1!=rows2){
        printf("\nError, cannot mutiply matrices due to a dimenstions mismatch\n");
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
    int** BT = extract_transpose(argv[2],cols1,&K);
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
    clock_t start3,end3;
    double cpu_time_used3;
    start3 = clock();
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
    end3 = clock();
    cpu_time_used3 = (((double)(end-start))/CLOCKS_PER_SEC);


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