#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <pthread.h>
#define NUM_THREADS 2

using namespace std;

typedef struct {
  int start;
  int stop;
  std::vector<std::string> *fileContent;
  int *uniqConterArray;
} NameSet;

void *evalUnique(void *vptr_value) {
  int start, stop;
  NameSet *p = vptr_value;

  struct NameSet *myNameSet;

  start = myNameSet->start;
  stop = myNameSet->stop;


}

int main() 
{
  std::vector<std::string> file_contents;
  std::string line;
  int lineNumber=0;
  int uniqCount=0;
  char *p;
  int k;

  ifstream inFile ("dist.female.first");
  ofstream outputFile ("results.txt");

  while ( std::getline(inFile,line) )
    file_contents.push_back(line);

  // Store number of lines in the file
  int size = (int)file_contents.size();
  int *uniqCounterArray = new int[size];

  // Init array for storing counts per uniq
  for(int i=0;i<size;i++) {
    uniqCounterArray[i] = 0;
  }

  std::vector<std::string>::iterator it = file_contents.begin();
  for(; it != file_contents.end();++it) {
    lineNumber++;
    string word = *it;
    word = word.substr(0,word.find(' '));
    int wordLength = (int)word.length();
    int origLength = (int)word.length();
    int containsDupe = 0;
    p = new char [wordLength+1];

    strcpy (p, word.c_str());
    for(int x=0;x<wordLength;x++) {
      for(int j=0;j<wordLength;j++) {
        if(x==j) {
          continue;
        }
        else if(* (p+x)==*(p+j)) {
            k=j;
            wordLength--;
            containsDupe=1;
            while(k < wordLength) {
              *(p+k)=*(p+k+1);
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
        //outputFile << "Line: " << lineNumber << " Word: " << word << " Dupe: " << containsDupe << " Unique: " << wordLength << endl;
        outputFile << word << endl;
        uniqCounterArray[lineNumber] = wordLength;
      }
    //printf("word read is: %s Dupe: %d Size: %d\n", word, containsDupe,origLength);
    //strcpy(word,"");
    delete[] p;
  }
  cout << "Number of names with unique chars: " << uniqCount << endl;
  outputFile.close();
  delete[] uniqCounterArray;
  return 0;
}
