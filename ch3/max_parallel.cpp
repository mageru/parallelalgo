#include "omp.h"
#include <stdio.h>

int array_int_max(int a[], int n) {
int maxes[4],i,j;
#pragma omp parallel shared(maxes,a) \
      firstprivate(n) private(j) num_threads(4)
{
#pragma omp single
{
    for (j=0; j<4; j++)
      maxes[j]=0;
}
#pragma omp for
for (i=0; i<n; i++) {
    j=omp_get_thread_num();
    printf("thread=%d index=%d\n", j, i);
    if (a[maxes[j]]<a[i])
      maxes[j]=i;
}
}
i=maxes[0];
for (j=1; j<4; j++)
   if (a[i]<a[maxes[j]])
     i=maxes[j];
return i;
}

int main(int argc, char *argv[]) {
int i, test[20]=
      {5,15,82,33,4,87,65,5,8,
       9,11,1,22,33,4,9,44,55,8,9};
printf("%d threads\n", omp_get_max_threads());
i=array_int_max(test,20);
printf("index=%d max=%d\n", i, test[i]);
return 0;
}
