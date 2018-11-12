#include <stdio.h>
#include "time.h"//clock()
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define SIZE 4
#define PROC 4
#define xMesh 2
#define sizeOfEach 2
//static const int SIZE=6;
//static const int PROC=4;

float array[SIZE][SIZE];
//static const int xMesh=2;//(int)sqrt(PROC);
//static const int sizeOfEach=3;//SIZE/(int)sqrt(PROC);
float arrayOfEach[sizeOfEach][sizeOfEach];
float L [sizeOfEach][sizeOfEach];//store L and U
float U [sizeOfEach][sizeOfEach];
float detOfEach;
float det [xMesh][xMesh]={{0}};
//float msg [xMesh][xMesh];
//float LRoot [xMesh][xMesh];//store L and U
//float URoot [xMesh][xMesh];


float Udet(int size,float a[size][size])
{
    int i;
    float det=1;
    for (i=0; i<size; i++)
    {
        det*=a[i][i];
    }
    return det;
}


float sum(int row,int column,int flag)
{
    float sum=0;
    int k=row;
    int j=column;
    int m;
    if(flag==1)
    {
        for (m=0; m<k; m++)
        {
            sum+=L[k][m]*U[m][j];
        }
    }
    else {
        for (m=0; m<j; m++)
        {
            sum+=L[k][m]*U[m][j];
        }
    }
    
    return sum;
}


float detArray1( int n, float a [SIZE][SIZE] )// n is the dimension // a is the array
{
    //int n=n;
    //  if (n==SIZE) {
    float b[SIZE][SIZE] = {{0}};
    //   }
    
    int i = 0, j = 0;
    float det = 0;
    int x = 0, c = 0, p = 0;
    if (n == 1)
        return a[0][0];
    for (i = 0; i <= n-1; i++)
    {
        for (c = 0; c < n - 1; c++)
        {
            for (j = 0; j < n - 1; j++)
            {
                if (c < i) {
                    p = 0;
                }
                else {
                    p = 1;
                }
                b[c][j] = a[c + p][j + 1];
            }
        }
        if (i % 2 == 0) {
            x = 1;
        }
        else {
            x = (-1);
        }
        float t=detArray1(n-1, b);
        det += a[i][0] * t * x;
    }
    return det;
}


int main(int argc, char ** argv){
    /*
     int n1,m1;
     
     for (n1=0;n1<xMesh;n1++)
     {
     for(m1=0;m1<xMesh;m1++)
     {
     LRoot[n1][m1]=0;
     if (m1==n1)
     {
     LRoot[m1][n1]=1;
     }
     URoot[n1][m1]=0;
     det[xMesh][xMesh]=0;
     //msg[xMesh][xMesh]=0;
     }
     }*/
    
    int myNode,allNodes;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &myNode);
    MPI_Comm_size( MPI_COMM_WORLD, &allNodes);
    
    
    
    // square matrix array
    if(myNode==0){
        int n,m,a;
        srand((unsigned)time(NULL));
        for (n=0;n<SIZE;n++)
        {
            for(m=0;m<SIZE;m++)
            {
                while (1) {
                    a=rand()%10;
                    if (a!=0) {
                        break;
                    }
                }
                array[n][m]=a;
                L[n][m]=0;
                if (m==n)
                {
                    L[m][n]=1;
                }
                U[n][m]=0;
            }
        }
        float ansDET=detArray1(SIZE, array);
        
        printf("ansDET0 %f\n",ansDET);
    }
    else{
        int n,m;
        for (n=0;n<SIZE;n++)
        {
            for(m=0;m<SIZE;m++)
            {
                L[n][m]=0;
                if (m==n)
                {
                    L[m][n]=1;
                }
                U[n][m]=0;
            }
        }
        
    }
    //all to all broadcast mesh
    int left,right,s;
    left=myNode-(myNode % xMesh)+(myNode-1)%xMesh;
    right=myNode-(myNode % xMesh)+(myNode+1)%xMesh;
    //for (s=1; s<=xMesh-1; s++)
    // {
    if (myNode%xMesh==0)
    {
        MPI_Send( &array, SIZE*SIZE, MPI_FLOAT, right, 0, MPI_COMM_WORLD );
        MPI_Recv( &array, SIZE*SIZE, MPI_FLOAT, left, 0, MPI_COMM_WORLD,&status );
    }
    else
    {
        MPI_Recv( &array, SIZE*SIZE, MPI_FLOAT, left, 0, MPI_COMM_WORLD,&status );
        MPI_Send( &array, SIZE*SIZE, MPI_FLOAT, right, 0, MPI_COMM_WORLD );
    }
    // }
    int up,down;
    up=(myNode-xMesh)%PROC;
    down=(myNode+xMesh)%PROC;
    //for (s=1; s<=xMesh-1; s++)
    //  {
    if(myNode<xMesh)
    {
        MPI_Send( &array, SIZE*SIZE, MPI_FLOAT, down, 0, MPI_COMM_WORLD );
        MPI_Recv( &array, SIZE*SIZE, MPI_FLOAT, up, 0, MPI_COMM_WORLD,&status );
    }
    else
    {
        MPI_Recv( &array, SIZE*SIZE, MPI_FLOAT, up, 0, MPI_COMM_WORLD,&status );
        MPI_Send( &array, SIZE*SIZE, MPI_FLOAT, down, 0, MPI_COMM_WORLD );
        
    }
    //  }
    
    int n,m;
    if (myNode==0) {
        for (n=0;n<SIZE;n++)
        {
            for(m=0;m<SIZE;m++)
            {
                printf("array is from node %d (n%d,m%d)=%f\n",myNode,n,m,array[n][m]);
                // printf("%f ",array[m][n]);
            }
            printf("\n");
        }
    }
    
    // caculate every array
    int x1,y1,x,y;//the begining of array// row column
    int s1,s2;
    
    x=myNode /xMesh;
    y=myNode % xMesh;
    x1=x*sizeOfEach;
    y1=y*sizeOfEach;
    //  printf("start from node %d (x=%d,y=%d)\n",myNode,x,y);
    //   printf("start from node %d (x1=%d,y1=%d)\n",myNode,x1,y1);
    
    for(s1=0;s1<=sizeOfEach-1;s1++)
    {
        for (s2=0; s2<sizeOfEach; s2++)
        {
            
            
            arrayOfEach[s1][s2]=array[x1][y1];
            //  printf("start from node %d (s1=%d,s2=%d),(x1=%d,y1=%d),arryOfEach%f\n",myNode,s1,s2,x1,y1,arrayOfEach[s1][s2]);
            y1++;
        }
        y1=y1-sizeOfEach;
        x1++;
    }
    for (n=0;n<sizeOfEach;n++)
    {
        for(m=0;m<sizeOfEach;m++)
        {
            printf("each is from node %d (n%d,m%d)=%f\n",myNode,n,m,arrayOfEach[n][m]);
            // printf("%f ",array[m][n]);
        }
        printf("\n");
    }
    
    // every det
    
    int k,j,i;
    for(i=0;i<=sizeOfEach-1;i++)
    {
        U[0][i]=arrayOfEach[0][i];
        L[i][0]=arrayOfEach[i][0]/U[0][0];
    }
    
    for (k=0;k<=sizeOfEach-1;k++)
    {
        
        for(j=k;j<=sizeOfEach-1;j++)
        {
            float s1;
            s1=sum(k,j,1);
            U[k][j]=arrayOfEach[k][j]-s1;
            if (k>=1) {
                float s2;
                s2=sum(j,k,2);
                L[j][k]=(arrayOfEach[j][k]-s2)/U[k][k];
            }
        }
    }
    
    detOfEach=Udet(sizeOfEach,U);
    // det[x][y]=detOfEach;
    // msg[x][y]=detOfEach;
    printf("det of U is %f, from node %d\n",detOfEach,myNode);
    MPI_Barrier(MPI_COMM_WORLD); //
    
    MPI_Gather(&detOfEach, 1, MPI_FLOAT,&det, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    for (n=0;n<xMesh;n++)
    {
        for(m=0;m<xMesh;m++)
        {
            printf("det array from node %d is (n%d,m%d)=%f\n",myNode,n,m,det[n][m]);
            // printf("%f ",array[m][n]);
        }
        printf("\n");
    }
    //  ansDET=detArray(xMesh, det);
    
    //  printf("ansDET %f\n",ansDET);
    /*
     //reduce
     // for (s=1; s<=(int)sqrt(PROC)-1; s++)
     //  {
     if (myNode%xMesh!=0)
     {
     MPI_Send( &msg, xMesh*xMesh, MPI_INT, left, 0, MPI_COMM_WORLD );
     MPI_Recv( &msg, xMesh*xMesh, MPI_INT, right, 0, MPI_COMM_WORLD,&status );
     /*for(i=x;i<xMesh;i++)
     {
     for(j=y;j<xMesh;j++)
     {
     det[i][j]=msg[i][j];
     }
     }
     for (i=0; i<xMesh; i++)
     {
     for(j=0;j<xMesh;j++)
     {
     msg[i][j]=det[i][j];
     }
     }
     */
    /* }
     else
     {
     MPI_Recv( &msg, xMesh*xMesh, MPI_INT, right, 0, MPI_COMM_WORLD,&status );
     MPI_Send( &msg, xMesh*xMesh, MPI_INT, left, 0, MPI_COMM_WORLD );
     
     }
     
     
     if(myNode>=xMesh)
     {
     MPI_Send( &msg, xMesh*xMesh, MPI_INT, up, 0, MPI_COMM_WORLD );
     MPI_Recv( &msg, xMesh*xMesh, MPI_INT, down, 0, MPI_COMM_WORLD,&status );
     
     }
     else
     {
     MPI_Recv( &msg, xMesh*xMesh, MPI_INT, down, 0, MPI_COMM_WORLD,&status );
     MPI_Send( &msg, xMesh*xMesh, MPI_INT, up, 0, MPI_COMM_WORLD );
     
     }
     
     for (n=0;n<xMesh;n++)
     {
     for(m=0;m<xMesh;m++)
     {
     printf("det array from node %d is (n%d,m%d)=%f\n",myNode,n,m,msg[n][m]);
     // printf("%f ",array[m][n]);
     }
     printf("\n");
     }
     */
    
    if (myNode==0)
    {
        /*
         for(i=0;i<=xMesh-1;i++)
         {
         URoot[0][i]=det[0][i];
         LRoot[i][0]=det[i][0]/URoot[0][0];
         }
         
         for (k=0;k<=xMesh-1;k++)
         {
         
         for(j=k;j<=xMesh-1;j++)
         {
         float s1;
         s1=sum(k,j,3);
         URoot[k][j]=det[k][j]-s1;
         if (k>=1) {
         float s2;
         s2=sum(j,k,4);
         LRoot[j][k]=(det[j][k]-s2)/URoot[k][k];
         }
         }
         }*/
        float ans=det[0][1]*(det[1][1]/det[0][1]*det[0][0]-det[1][0]);
        printf("FIND_DET is %f, from node %d\n",ans,myNode);
        
        //  float ansDET1=detArray2(xMesh, det);
        
        //   printf("ansDET %f\n",ansDET1);
    }
    
    MPI_Finalize();
    
    return 0;
}
//}

