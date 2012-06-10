#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>
#define NUM_THREAD 2
#define batch 100
using namespace std;

std::vector<std::string> fileContent;


struct SplitSet{
  int start;
  int end;
};

struct SplitSet thread_data_array[NUM_THREAD];

void *threadVector(void *vptr_value) {
  int start,end;
  
  struct SplitSet *mySet;
  mySet = (struct SplitSet *)vptr_value;

  start = mySet -> start;
  end = mySet -> end;

  for (int i=start-1;i<end; i++) {
    //printf("String is: %s", fileContent[start]);
    cout << fileContent[start] << std::endl;
    start++;
  }

  return (void *) 0;
}


int main() {
  pthread_t firstSetThread;
  pthread_t secondSetThread;

  //std::vector<std::string> fileContent;
  fileContent.push_back("Hello");
  fileContent.push_back("my");
  fileContent.push_back("name");
  fileContent.push_back("is"); 
  fileContent.push_back("Justin");

  std::vector<std::string>::iterator it = fileContent.begin();
  //for(; it != fileContent.end();++it) {
  //  string word = *it;
  //  cout << "Word is : " << word << endl;
  //}
  thread_data_array[1].start = 0;
  thread_data_array[1].end = 2;
  thread_data_array[2].start = 3;
  thread_data_array[2].end = 4;

  pthread_create(&firstSetThread, NULL, threadVector, (void *) &thread_data_array[1]);
  pthread_create(&secondSetThread, NULL, threadVector, (void *) &thread_data_array[2]);

  pthread_join(firstSetThread,NULL);
  pthread_join(secondSetThread,NULL);

  return 0;
}

