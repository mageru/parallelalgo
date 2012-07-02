#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define __SSE__
#include "vVector.h"
#define N 20000000
int array_int_max(vInt32 a[], int n) {
  int i;
  vInt32 max,temp,temp1;
  vCopy(max,a[0]);
  for (i=1; i<n; i+=4) {
        vMax_int(temp,a[i],a[i+1]);
            vMax_int(temp1,a[i+2],a[i+3]);
                vMax_int(max,temp,max);
                    vMax_int(max,temp1,max);
  }
  vSplat_int(temp,max,0);  vMax_int(max,temp,max);
  vSplat_int(temp,max,1);  vMax_int(max,temp,max);
  vSplat_int(temp,max,2);  vMax_int(max,temp,max);
  return vExtract_int(max,3);
}

int test[N];
int main(int argc, char *argv[]) {
  int i,j;
  for (i=0; i<N; i++)
      test[i]=rand();
  j=clock();
  i=array_int_max((vInt32 *)test,N/4);
  printf("clock=%d max=%d\n", (int)clock()-j, i);
  return 0;
}
