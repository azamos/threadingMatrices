#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#define CHUNCK_SIZE 8

int** extract_matrix( const char* fileName,int* numRows, int* numCols){
    FILE* file = fopen(fileName,"r");
    int num;
    int row_counter = 0;
    int ROWS = CHUNCK_SIZE;
    int** matrix = (int**)malloc(sizeof(int *)*ROWS);
    int j = 0;
    int COLS;
    short COLS_IS_SET = 0;
    
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

int main(int argc, char* argv[]){
    if(argc<3){
        printf("\nMissing source file for matrix 1 and source file for matrix 2");
        exit(-1);
    }
    printf("\nmatrix1 file: %s,matrix2 file: %s",argv[1],argv[2]);
    int rows1,cols1,rows2,cols2;
    int** matrix1 = extract_matrix(argv[1],&rows1,&cols1);
    int** matrix2 = extract_matrix(argv[2],&rows2,&cols2);
    
    printf("\nmatrix1 has %d rows and %d cols\n",rows1,cols1);
    for(int i = 0; i< rows1; i++){
        printf("\n");
        for(int k = 0; k< cols1; k++){
            printf("%d ",matrix1[i][k]);
        }
    }
    printf("\nmatrix2 has %d rows and %d cols\n",rows2,cols2);
    for(int i = 0; i< rows2 ;i++){
        printf("\n");
        for(int k = 0; k< cols2; k++){
            printf("%d ",matrix2[i][k]);
        }
    }
    if(cols1!=rows2){
        printf("\nError, cannot mutiply matrices due to a dimenstions mismatch\n");
        free_matrix(matrix1,rows1);
        free_matrix(matrix2,rows2);
        exit(-1);
    }
    printf("\nmatrix1 and matrix 2 can be mutliplied!\n");
    /*First, I will do multiplication WITHOUT multi-threading, and time it*/
    
    free_matrix(matrix1,rows1);
    free_matrix(matrix2,rows2);
    return 0;
}