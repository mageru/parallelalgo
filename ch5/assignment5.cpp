// Justin Miller
// July 1st 2012
// Assignment 5 - Concurrent Programming

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>

typedef struct {
    int x, y;
} NumberPair;

const int MAX = 1000;
const int INTERCHANGES = 100;

int incrementedArray[MAX];
int verifyArray[MAX];

void *threaded_merge_sort(void *vptr_value) {
    NumberPair range = *(NumberPair *)vptr_value, lrange, rrange;
    pthread_t lside, rside;
    int lleft, n1, n2, i, y, k;
    int lright = range.x;
    int rleft = range.y;
    int n = rleft - lright + 1;
    int *aleft, *aright;

    if (lright < rleft) {
        lleft = (lright + rleft) >> 1;
        long id = pthread_self();
        printf("id: %ld lright: %d lleft: %d rright: %d rleft: %d\n", id, lright, lleft, (lleft + 1), rleft);
        lrange.x = lright, lrange.y = lleft, rrange.x = lleft + 1, rrange.y = rleft;
        pthread_create(&lside, NULL, threaded_merge_sort, (void *)&lrange);
        pthread_join(lside, NULL);
        pthread_create(&rside, NULL, threaded_merge_sort, (void *)&rrange);
        pthread_join(rside, NULL);

        n1 = lleft - lright + 1;
        n2 = rleft - lleft;
        aleft = (int *)malloc(sizeof(int) * n1);
        aright = (int *)malloc(sizeof(int) * n2);
        for (i = 0; i < n1; i++) {
            aleft[i] = incrementedArray[lright + i];
        }
        for (i = 0; i < n2; i++) {
            aright[i] = incrementedArray[lleft + 1 + i];
        }
        for (k = i = y = 0; k < n; k++) {
            if (i >= n1 || (y < n2 && aleft[i] > aright[y])) {
                incrementedArray[k + lright] = aright[y++];
            }
            else {
                incrementedArray[k + lright] = aleft[i++];
            }
        }
        free(aleft);
        free(aright);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
// Initialize array of numbers 1 to 1,000,000
    printf("Populating Array of size: %d\n", MAX);
    for (int i = 0; i < MAX; i += 5) {
        incrementedArray[i] = i + 1;
        verifyArray[i] = i + 1;
        incrementedArray[i + 1] = i + 2;
        verifyArray[i + 1] = i + 2;
        incrementedArray[i + 2] = i + 3;
        verifyArray[i + 2] = i + 3;
        incrementedArray[i + 3] = i + 4;
        verifyArray[i + 3] = i + 4;
        incrementedArray[i + 4] = i + 5;
        verifyArray[i + 4] = i + 5;
        //printf("Value: %d to verifyArray location: %d, ",verifyArray[i],i);
        //printf("Value: %d to verifyArray location: %d, ",verifyArray[i+1],(i+1));
        //printf("Value: %d to verifyArray location: %d, ",verifyArray[i+2],(i+2));
        //printf("Value: %d to verifyArray location: %d, ",verifyArray[i+3],(i+3));
        //printf("Value: %d to verifyArray location: %d, ",verifyArray[i+4],(i+4));
        printf("Value: %d to incrementedArray location: %d\n, ", incrementedArray[i], i);
        printf("Value: %d to incrementedArray location: %d\n, ", incrementedArray[i + 1], (i + 1));
        printf("Value: %d to incrementedArray location: %d\n, ", incrementedArray[i + 2], (i + 2));
        printf("Value: %d to incrementedArray location: %d\n, ", incrementedArray[i + 3], (i + 3));
        printf("Value: %d to incrementedArray location: %d\n, ", incrementedArray[i + 4], (i + 4));
    }

//// Do random interchanges
    printf("Interchanging %d values in array.\n", INTERCHANGES);
    for (int x = 0; x < INTERCHANGES; x++) {
        long swapOne = rand() % MAX;
        long swapTwo = rand() % MAX;
        long curSwapOne = incrementedArray[swapOne];
        incrementedArray[swapOne] = incrementedArray[swapTwo];
        incrementedArray[swapTwo] = curSwapOne;
        printf("Array location: %ld  Value: %d\n", swapOne, incrementedArray[swapOne]);
        printf("Array location: %ld  Value: %d\n", swapTwo, incrementedArray[swapTwo]);
        printf("\n");
    }
//int n,i;
    pthread_t sorter;
    NumberPair range;
    range.x = 1, range.y = MAX - 1;
    long start = (long)clock();
    pthread_create(&sorter, NULL, threaded_merge_sort, (void *)&range);
    pthread_join(sorter, NULL);
    long runtime = (long)clock - start;
    bool verified = true;
    for (int i = 0; i < MAX; i++) {
        printf("Found: %d Location: %d Expected: %d Location: %d\n", incrementedArray[i], i, verifyArray[i], i);
        if (incrementedArray[i] != verifyArray[i]) {
            verified = false;
            printf("ERROR: Array not in sorted order. \n-- Found: %d at %d. Expected: %d\n", incrementedArray[i], i, verifyArray[i]);
            exit(-1);
        }
    }
    if (verified) {
        printf("Number of Elements: %d\n", MAX);
        printf("Interchanges Made: %d\n", INTERCHANGES);
        printf("Algorithm: Merge Sort\n");
        printf("Array sorted = verified. Clock=%ld\n", runtime);
    }
    printf("\n");
//}
    return 0;
}


