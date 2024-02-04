#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#define CHUNCK_SIZE 8
#define BILLION 1000000000L

typedef struct {
    int** A;/*MxN*/
    int** BT;/*KxN*/
    int** AB;/*MxK*/
    int N;
    int K;
    int start;
    int end;
} ThreadData;

int** extract_transpose( const char* fileName, int N, int*numCols){
    /*Explanation about numRows: if matrix A dimensions are MxN, then matrix B's dimensions are NxK.
    Meaning, this function assumes that matrix A and its dimensions were already extracted. All that is left to do here,
    is to discover how many colums there are. Those will become the rows, and the numRows will serve as colums here*/
    FILE* file = fopen(fileName,"r");
    int num;
    int row_counter = 0;
    int a = 0;
    int K = 0;
    while(fscanf(file,"%d",&num)==1){
        a++;
        if(fgetc(file)=='\n'){
            K = a;
            break;
        }
    }
    fclose(file);
    /*Extracted K*/
    *numCols = K;
    int** transpose = (int**)malloc(sizeof(int*)*K);
    for(int i =0; i<K;i++){
        transpose[i] = (int*)malloc(sizeof(int)*N);
    }

    file = fopen(fileName,"r");
    int i =0;
    int j = 0;
    while(fscanf(file,"%d",&num)==1){
        transpose[j++][i] = num;
        if(fgetc(file)=='\n'){
            i++;
            j=0;
        }
    }
    fclose(file);
    return transpose;
}

int** extract_matrix( const char* fileName,int* numRows, int* numCols){
    FILE* file = fopen(fileName,"r");
    int num;
    int row_counter = 0;
    int ROWS = CHUNCK_SIZE;
    int** matrix = (int**)malloc(sizeof(int *)*ROWS);
    int j = 0;
    int COLS;
    short COLS_IS_SET = 0;
    /*this part is used to determine how many columns there are
    It does this by going over the numbers of the first line*/
    int a = 0;
    while(fscanf(file,"%d",&num)==1){
        a++;
        if(fgetc(file)=='\n' && COLS_IS_SET == 0){
            COLS = a;
            COLS_IS_SET = 1;
            break;
        }
    }
    matrix[0] = (int*)malloc(sizeof(int)*COLS);
    fclose(file);
    /*here we scan the number from the file into the matrix*/
    file = fopen(fileName,"r");
    while(fscanf(file,"%d",&num)==1){
        matrix[row_counter][j++] = num;
        if(fgetc(file)=='\n'){
            j=0;//reseting the column index
            row_counter++;
            if(row_counter > ROWS){
                ROWS *= 2;
                matrix = (int**)realloc(matrix,sizeof(int *)*ROWS);
            }
            matrix[row_counter] = (int*)malloc(sizeof(int)*COLS);
        }
    }
    fclose(file);
    *numCols = COLS;
    *numRows = row_counter+1;
    return matrix;
}

void free_matrix(int** matrix,int rows){
    for(int i = 0; i< rows; i++){
        free(matrix[i]);
    }
    free(matrix);
}
void print_matrix(int** matrix, int M, int N){
    for(int i =0; i< M; i++){
        printf("\n");
        for(int j=0; j< N; j++){
            printf("%d ",matrix[i][j]);
        }
    }
}
int** multiply_matrices(int** A, int rowsA,int colsA, int** B, int rowsB, int colsB){
    if(colsA!=rowsB){
        printf("\nError, cannot mutiply matrices due to a dimenstions mismatch\n");
        return NULL;
    }
    /*First, allocating space for result: dimensions are rowsA X colsB*/
    int** AB = (int**)malloc(sizeof(int *)*rowsA);
    for(int i = 0; i< rowsA; i++){
        AB[i] = (int*)malloc(sizeof(int)*colsB);
    }
    for(int i = 0; i < rowsA; i++){
        for(int j = 0; j< colsB;j++){
            int sum = 0;
            for(int k = 0; k< colsA; k++){
                sum += A[i][k]*B[k][j];
            }
            AB[i][j] = sum;
        }
    }
    return AB;
}

int** modified_multiply(int** A, int rowsA, int colsA, int** BT, int rowsBT, int colsBT){
    /*if A is MxN, and B is NxK(and BT is KxN), --> AB is MxK.
    So, first I allocate M = rowsA rows.Then, for each row, I will allocate K = rowsBT columns*/
    int** AB = (int**)malloc(sizeof(int *)*rowsA);
    for(int i =0; i< rowsA; i++){
        AB[i] = (int*)malloc(sizeof(int)*rowsBT);
    }
    /*Next, the modified multiplication*/
    for(int i =0; i< rowsA; i++){
        for(int j =0; j<rowsBT;j++){
            int sum = 0;
            for(int k = 0;k<colsBT;k++){
                sum+=A[i][k]*BT[j][k];
            }
            AB[i][j] = sum;
        }
    }
    return AB;
}


int get_threads_amount(){
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores < 1) {
        fprintf(stderr, "Error retrieving the number of available cores.\n");
        return 1;
    }
    printf("Number of available cores: %ld\n", num_cores);
    return num_cores;
}

void* process_block(void* arg){
    ThreadData* data = (ThreadData*)arg;
    printf("\nentered process_block... start = %d and end = %d\n",data->start,data->end);
    for(int i = data->start; i<data->end; i++){
        for(int j = 0; j < data->K ; j++){
            data->AB[i][j]= 0;
            for(int l = 0; l < data->N; l++){
                // printf("\nHERE %d \n",data->AB[i][j]);
                data->AB[i][j] += data->A[i][l]*(data->BT[j][l]);
                // printf(" %d",data->AB[i][j]);
            }
            //printf("\nsum = %d\n",data->AB[i][j]);
        }
    }
}


int main(int argc, char* argv[]){
    if(argc<3){
        printf("\nMissing source file for matrix 1 and source file for matrix 2");
        exit(-1);
    }
    int rows1,cols1,rows2,cols2;
    int** matrix1 = extract_matrix(argv[1],&rows1,&cols1);
    int** matrix2 = extract_matrix(argv[2],&rows2,&cols2);

    if(cols1!=rows2){
        printf("\nError, cannot mutiply matrices due to a dimenstions mismatch\n");
        free_matrix(matrix1,rows1);
        free_matrix(matrix2,rows2);
        exit(-1);
    }
    int K;
    int** BT = extract_transpose(argv[2],cols1,&K);

    /*Generated transpose from file. Now, to try modified matrix mult*/
    int cores=get_threads_amount();
    printf("\nNow to try a %d threaded matrix multiplication\n",cores);

    pthread_t* threads = (pthread_t*)malloc(cores*sizeof(pthread_t));
    
    int work_qouta = cols1 / cores;
    printf("work qouta = %d  ",work_qouta);
    int remaining = cols1 % cores;
    ThreadData** datas = (ThreadData**)malloc(cores*sizeof(ThreadData*));
    int** AB3 = (int**)malloc(rows1*sizeof(int*));
    for(int i =0; i <  rows1; i++){
        AB3[i] = (int*)malloc(cols2*sizeof(int));
    }

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

    for(int i =0; i< rows1;i++){
        printf("\n\n");
        for(int j =0; j<K; j++ ){
            printf("AB3[%d][%d] = %d",i,j,AB3[i][j]);
            printf("\n");
        }
    }
    
    free_matrix(matrix1,rows1);
    free_matrix(matrix2,rows2);
    free_matrix(BT,cols1);
    free_matrix(AB3,cols1);

    free(threads);
    for(int i = 0; i < cores; i++){
        free(datas[i]);
    }
    free(datas);
    return 0;
}