#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 1008
int A[N][N], B[N][N], C[N][N];
int main(int argc, char *argv[]) {
int i,j,k,z;
for (i=0; i<N; i++)
for (j=0; j<N; j++) {
    A[i][j]=rand();
    B[i][j]=A[i][j]+1;
    C[i][j]=A[i][j]-1;
}
z=clock();
for (i=0; i<N; i+=4)
for (j=0; j<N; j++)
for (k=0; k<N; k++) {
    //8301 clocks, no unrolling
    A[i][j] = A[i][j] + B[i][k] * C[k][j];
    //4281 clocks, 2 statements            
    A[i+1][j] = A[i+1][j] + B[i+1][k] * C[k][j];
    //3251 clocks, 3 statements
    A[i+2][j] = A[i+2][j] + B[i+2][k] * C[k][j];
    //3063 clocks, 4 statements
    A[i+3][j] = A[i+3][j] + B[i+3][k] * C[k][j];
}
printf("clock=%d\n", (int)clock()-z);
return 0;
}
