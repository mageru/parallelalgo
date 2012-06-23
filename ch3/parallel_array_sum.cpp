#include "omp.h"
#include <stdio.h>
#include <math.h>
#define M 3
#define N 3
int main (int argc, char *argv[]) {
int a[M][N];
int i,j,sum=0,k=1;
for (i=0; i<M; i++)
   for (j=0; j<N; j++)
     a[i][j]=k++;
#pragma omp parallel for \
   reduction(+ : sum) private(j)
for (i=0; i<M; i++)
  for (j=0; j<N; j++) {
    sum+=a[i][j];
    printf("thread=%d i=%d j=%d\n",
             omp_get_thread_num(),i,j);
  } //for j
printf("sum= %d should be=%d\n",
              sum, M*N*(M*N+1)/2);
return 0;
}
