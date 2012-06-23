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
pthread_mutex_t mutexOutput;
ofstream outputFile ("results.txt");


typedef struct {
  int start,end,threadCounter;
} SplitSet;

void *threadVector(void *vptr_value) {
  int start,end,threadCounter,counter,k,uniqCount;
  char *wordArray;
  counter = 0;
  uniqCount = 0;
  
  SplitSet *mySet;
  mySet = (SplitSet *)vptr_value;

  start = mySet -> start;
  end = mySet -> end;
  threadCounter = mySet -> threadCounter;

  while(start < end) {
    string word = file_contents[start];
    word = word.substr(0,word.find(' '));
    int wordLength = (int)word.length();
    int origLength = (int)word.length();
    int containsDupe = 0;
    wordArray = new char [wordLength+1];
    strcpy (wordArray, word.c_str());
    for(int x=0;x<wordLength;x++) {
      for(int j=0;j<wordLength;j++) {
        if(x==j) {
          continue;
        }
        else if(* (wordArray+x)==*(wordArray+j)) {
            k=j;
            wordLength--;
            containsDupe=1;
            while(k < wordLength) {
              *(wordArray+k)=*(wordArray+k+1);
              k++;
            }
            j=0;
        }
      }
    }
    if(containsDupe == 0) {
      uniqCount++;
    }
    if(containsDupe == 0 && origLength >= 6) {
      pthread_mutex_lock (&mutexOutput);
        outputFile << word << endl;
      pthread_mutex_unlock (&mutexOutput);
      //cout << word << endl;
      //uniqCounterArray[lineNumber] = wordLength;
    }
    delete[] wordArray;
    start++;
    counter++;
  }
  
  
  //cout << "Number of names with unique chars: " << uniqCount << endl;
  cout << "Thread: " << threadCounter << " Processed: " << counter << " Unique: " << uniqCount << endl;
  //pthread_exit(NULL);
  return (void *) uniqCount;
}


int main() {
  pthread_t threads[25];
  pthread_attr_t attr;
  SplitSet sets[25];
  int totalUniq=0;

  pthread_mutex_init(&mutexOutput, NULL);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  
  int lineCount,splitCount,remainder;

  string line;
  ifstream inFile ("dist.female.first");
  while(std::getline(inFile,line)) {
    file_contents.push_back(line);
  }
  inFile.close();

  int uniqWordTracker[file_contents.size()];

  lineCount = file_contents.size()-1;
  splitCount = lineCount / 25;
  remainder = lineCount % 25;
  //cout << "Line count is: " << lineCount << " Split Count: " << splitCount << " Remainder: " << remainder << endl;

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
    //cout << "The iteration is: " << i << endl;
    assert(
      pthread_create(&threads[i], &attr, threadVector, (void *) &sets[i]) == 0);
    //cout << "Start: " << sets[i].start << " End: " << sets[i].end << " Thread: " << threadCounter << endl;
    threadCounter++;
    lineCounter = lineCounter + splitCount + 1;
  }
  
  while (--threadCounter >= 0) {
    int uniqPerThread;
    //cout << "Thread counter: " << threadCounter << endl;
    assert(pthread_join(threads[threadCounter], (void**)&uniqPerThread) == 0);
    totalUniq += uniqPerThread;
    //cout << "Joining: " << sets[threadCounter].threadCounter << endl;
  }
  pthread_mutex_destroy(&mutexOutput);
  outputFile.close();
  cout << "Total Unique: " << totalUniq << endl;
  return 0;
}

