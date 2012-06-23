#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 20000000
int array_int_max(int a[], int n) {
    int i, max=0;
    for (i=1; i<n; i++)
        if (a[max]<a[i])
          max=i;
    return max;
}

int test[N];
int main(int argc, char *argv[]) {
int i, j;
for (i=0; i<N; i++)
  test[i]=rand();
j=clock();
i=array_int_max(test,N);
printf("clock=%ld index=%d max=%d\n",
                                 clock()-j, i, test[i]);
return 0;
}
