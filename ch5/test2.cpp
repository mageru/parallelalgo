#include <string>
#include <unistd.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <semaphore.h>
#include <pthread.h>
#include <cstdlib>
#define THREADS_NUM 2
const int MAX = 200;

using namespace std;

void swap(int a, int b);

/// global variables
sem_t mysemaphore[MAX];
int list[MAX];
int n=0;
bool sorted=false;


void *sort_thread(void *param)  
{
  int type_ = *(int *)param;
  bool sorted = false;

  while(!sorted)
  {
  sorted=true;
  // odd-even
  for(int i=type_;i<n-1;i+=2)
  {
    if(list[i]>list[i+1])
    {
    // lock required resourcse and preform swap, set sorted to false so the thread loops
    sem_wait(&mysemaphore[i]);
    sem_wait(&mysemaphore[i+1]);
    swap(i,i+1);
    sorted=false;
    sem_post(&mysemaphore[i]);
    sem_post(&mysemaphore[i+1]);
    }
  }
  }
//pthread_exit(NULL);  don't know if needed
}

int main(int argc,char* argv[])
{
for(int i=0;i<MAX;i++) {
  list[i]=i;
  n++;
}
for(int x=0;x<10;x++) {
  int swapOne = rand() % 200;
  int swapTwo = rand() % 200;
  int curSwapOne = list[swapOne];
  int curSwapTwo = list[swapTwo];
  list[swapOne] = curSwapTwo;
  list[swapTwo] = curSwapOne;
  printf("Array location: %d  Value: %d\n",swapOne,list[swapOne]);
  printf("Array location: %d  Value: %d\n",swapTwo,list[swapTwo]);
  printf("\n");
}

int temp;

/// make semaphore for each item in list to lock that resource
for (int i=0; i<MAX; i++)
{
sem_init(& mysemaphore[i],0,1);
}

pthread_t threads[THREADS_NUM];

// spawn two threads
for(int i=0; i<THREADS_NUM; i++)
{
  pthread_create(&threads[i], NULL, sort_thread, (void *)&i);
}

// not sure if i need to join but...
for (int count = 0; count < 2; count++)
{
  pthread_join(threads[count], NULL);
}


/// display sorted elements
for(int i=0;i<n;i++)
{
  cout << list[i] << endl;
}

return 0;
}

// swap function
void swap(int a, int b)
{
  if (list[a] > list[b])
  {
  int temp=list[a];
  list[a]=list[b];
  list[b]=temp;
  }
}

