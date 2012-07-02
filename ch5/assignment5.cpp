#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>

typedef struct {
  int x,y;
} NumberPair;

const int MAX = 100;
const int INTERCHANGES = 10;

int incrementedArray[MAX];
int verifyArray[MAX];

void *threaded_merge_sort(void *param) {
    NumberPair range = *(NumberPair *)param, lrange, rrange;
    pthread_t lside, rside;
    int p = range.x, q, r = range.y, n1, n2, n = r-p+1, i, y, k;
    int *aleft, *aright;
    if(p < r) {
        q = (p + r) >> 1;
        int id = pthread_self();
        printf("id: %d p: %d q: %d q+1: %d r: %d\n",id,p,q,(q+1),r);
        lrange.x = p, lrange.y = q, rrange.x = q + 1, rrange.y = r;
        pthread_create(&lside, NULL, threaded_merge_sort, (void *)&lrange);
        pthread_join(lside, NULL);
        pthread_create(&rside, NULL, threaded_merge_sort, (void *)&rrange);
        pthread_join(rside, NULL);

        n1 = q - p + 1;
        n2 = r - q;
        aleft = (int *)malloc(sizeof(int) * n1);
        aright = (int *)malloc(sizeof(int) * n2);
        for(i = 0; i < n1; i++) {
          aleft[i] = incrementedArray[p+i];
        }
        for(i = 0; i < n2; i++) {
          aright[i] = incrementedArray[q+1+i];
        }
        for(k = i = y = 0; k < n; k++) {
            if(i >= n1 || (y < n2 && aleft[i] > aright[y])) {
              incrementedArray[k+p] = aright[y++];
            }
            else {
              incrementedArray[k+p] = aleft[i++];
            }
        }
        free(aleft);
        free(aright);
    }
    pthread_exit(NULL);
}

int main(int argc,char *argv[]) {

// Initialize array of numbers 1 to 1,000,000
for(int i=1;i<MAX+1;i++) {
  incrementedArray[i]=i;
  verifyArray[i]=i;
  printf("Current val: %d\n", i+1);
}

//// Do random interchanges
for(int x=0;x<INTERCHANGES;x++) {
  int swapOne = rand() % MAX+1;
  int swapTwo = rand() % MAX+1;
  int curSwapOne = incrementedArray[swapOne];
  incrementedArray[swapOne] = incrementedArray[swapTwo];
  incrementedArray[swapTwo] = curSwapOne;
  printf("Array location: %d  Value: %d\n",swapOne,incrementedArray[swapOne]);
  printf("Array location: %d  Value: %d\n",swapTwo,incrementedArray[swapTwo]);
  printf("\n");
}
int n,i;
pthread_t sorter;
NumberPair range;
//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
//while(scanf("%d", &n)==1 && n) {
//for(i = 0; i < n; i++) {
//  scanf("%d", &incrementedArray[i]);
//}
//for(int j=0;j<2;j++){
range.x = 1, range.y = MAX;
long start = (long)clock();
pthread_create(&sorter, NULL, threaded_merge_sort, (void *)&range);
pthread_join(sorter, NULL);
long runtime = (long)clock-start;
bool verified = true;
for(i = 0; i < MAX; i++) {
  //printf("%d,", incrementedArray[i]);
  if(incrementedArray[i] != verifyArray[i]) {
    verified = false;
    printf("ERROR: Array not in sorted order. \n-- Found: %d at %d. Expected: %d\n",incrementedArray[i],i,verifyArray[i]);
    exit(-1);
  }
}
if(verified) {
  printf("Array sort verified. Clock=%ld\n",runtime);
}
printf("\n");
//}
return 0;
}


