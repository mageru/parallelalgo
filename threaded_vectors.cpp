#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <assert.h>
using namespace std;

std::vector<std::string> file_contents;


typedef struct {
  int start,end,threadCounter;
} SplitSet;

void *threadVector(void *vptr_value) {
  int start,end,threadCounter,counter;
  counter = 0;
  
  SplitSet *mySet;
  mySet = (SplitSet *)vptr_value;

  start = mySet -> start;
  end = mySet -> end;
  threadCounter = mySet -> threadCounter;

  for (int i=start;i<end; i++) {
    //printf("String is: %s", fileContent[start]);
    //cout << file_contents[start] << std::endl;
    start++;
    counter++;
  }
  cout << "Processed: " << counter << " Thread: " << threadCounter << endl;
  pthread_exit(NULL);
}


int main() {
  pthread_t threads[25];
  pthread_attr_t attr;
  SplitSet sets[25];

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
  int lineCount,splitCount,remainder;

  string line;
  ifstream inFile ("dist.female.first");
  while(std::getline(inFile,line)) {
    file_contents.push_back(line);
  }
  inFile.close();
  cout << "Line 4274: " << file_contents[4274] << endl;

  lineCount = file_contents.size()-1;
  splitCount = lineCount / 25;
  remainder = lineCount % 25;
  cout << "Line count is: " << lineCount << " Split Count: " << splitCount << " Remainder: " << remainder << endl;

  int lineCounter = 0;
  int threadCounter = 0;
  for(int i=0;i<=24;i++) {
    int endCounter = lineCounter + splitCount;
    sets[i].start = lineCounter;
    if(endCounter >= lineCount) {
      sets[i].end = lineCount;
    } 
    else {
      sets[i].end = lineCounter + splitCount;
    }
    sets[i].threadCounter = threadCounter;
    cout << "The iteration is: " << i << endl;
    assert(
      pthread_create(&threads[i], &attr, threadVector, (void *) &sets[i]) == 0);
    cout << "Start: " << sets[i].start << " End: " << sets[i].end << " Thread: " << threadCounter << endl;
    threadCounter++;
    lineCounter = lineCounter + splitCount + 1;
  }
  
  while (--threadCounter >= 0) {
  cout << "Thread counter: " << threadCounter << endl;
    assert(pthread_join(threads[threadCounter], NULL) == 0);
    cout << "Joining: " << sets[threadCounter].threadCounter << endl;
  }
  return 0;
}

