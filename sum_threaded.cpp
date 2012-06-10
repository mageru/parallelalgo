#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#define NUM_THREADS 2

struct SplitSet {
  int thread_id;
  int *set;
  int size;
};

struct SplitSet thread_data_array[NUM_THREADS];

void *sumSet(void *vptr_value) {
  int size,sum;
  sum = 0;

  struct SplitSet *mySet;

  mySet = (struct SplitSet *) vptr_value;
  size = mySet->size;
  int *setValues = (int *)mySet->set;

  //printf("Size: %d \n", size);
  for (int count = 0; count<size; count++) {
    //printf("Value: %d \n", setValues[count]);
    sum+=setValues[count];
  }

  //printf("Sum: %d \n",sum);
  return (void *) sum;
}

int main(int argc,char *argv[]) {
  pthread_t firstSetThread;      
  pthread_t secondSetThread;      
  void *r;     int i,totalSum;
  int sumFirst,sumSecond,rc;

  int midpoint = (argc-1)/2;
  midpoint +- 1;
  //printf("Midcount: %d \n", midpoint);

  int *firstSet = new int[(argc-1)/2];
  int *secondSet = new int[(argc-1)/2];

  int firstCounter = 0;
  int secondCounter= 0;

  for(int i=0;i<argc-1;i++) {
    if(i <= midpoint-1) {
      firstSet[firstCounter]=atoi(argv[i+1]);
      //printf("Value in firstSet: %d \n", firstSet[firstCounter]);
      firstCounter++;
    }
    if(i > midpoint-1) {
      secondSet[secondCounter]=atoi(argv[i+1]);
      //printf("Value in secondSet: %d \n", secondSet[secondCounter]);
      secondCounter++;
    }

  }

  thread_data_array[1].set = firstSet;
  thread_data_array[1].size = firstCounter;
  thread_data_array[1].thread_id = 1;
  thread_data_array[2].set = secondSet;
  thread_data_array[2].size = secondCounter;
  thread_data_array[2].thread_id = 2;

  
  rc = pthread_create(&firstSetThread, NULL, sumSet, (void *) &thread_data_array[1]);

  rc = pthread_create(&secondSetThread, NULL, sumSet, (void *) &thread_data_array[2]);

  if(rc) {
    printf("ERR: pthread_create() ret = %d\n",rc);
    exit(-1);
  } 

  pthread_join(firstSetThread,(void**)&sumFirst);

  totalSum += sumFirst;
  printf("Thread 1 return is: %d \n", sumFirst);
  
  pthread_join(secondSetThread,(void**)&sumSecond);

  delete[] firstSet;
  delete[] secondSet;

  printf("Thread 2 return is: %d \n", sumSecond);
  totalSum += sumSecond;
  printf("TOTAL SUM: %d \n", totalSum);

  return 0;
}

