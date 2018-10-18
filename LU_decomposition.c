#include <stdio.h>
#include "mpi.h"
#include "time.h"//clock()函数获取系统时间

#define SIZE 10
int array[SIZE][SIZE];
int L[SIZE][SIZE];
int U[SIZE][SIZE];

int main(int argc, char ** argv){
    int n,m;
    
    // square matrix array
    for (n=0;n<SIZE;n++)
    {
        for(m=0;m<SIZE;m++)
        {
            array[n][m]=n+1;
            L[n][m]=0;
            U[n][m]=0;
        }
    }
    
    int myNode,allNodes;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &myNode);
    MPI_Comm_size( MPI_COMM_WORLD, &allNodes);
   
    
   
    
    MPI_Finalize();
    return 0;
}
