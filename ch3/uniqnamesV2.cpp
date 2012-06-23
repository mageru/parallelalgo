#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <omp.h>
//#include <pthread.h>
//#define CHUNK 20
//#define NUM_THREADS 2

using namespace std;

typedef struct {
  int start;
  int end;
  std::vector<std::string> *fileContent;
} NameSet;

int main() 
{
  std::vector<std::string> file_contents;
  std::string line;
  int lineNumber=0;
  int uniqCount=0;
  char *p;
  int k,x,j;

  ifstream inFile ("dist.female.first");
  ofstream outputFile ("results.txt");

  while ( std::getline(inFile,line) )
    file_contents.push_back(line);

  // Store number of lines in the file
  int size = (int)file_contents.size();
  int counterUniq=0;
  std::vector<std::string>::iterator it = file_contents.begin();
  int lineCount = file_contents.size()-1;
  #pragma omp parallel for private (counterUniq)
  for(int i=0;i<lineCount;i++) {
    string word = *it;
    word = word.substr(0,word.find(' '));
    int wordLength = (int)word.length();
    int origLength = (int)word.length();
    int containsDupe = 0;
    p = new char [wordLength+1];

    strcpy (p, word.c_str());
    for(x=0;x<wordLength;x++) {
      for(j=0;j<wordLength;j++) {
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
        counterUniq++;
      }
      if(containsDupe == 0 && origLength >= 6) {
        outputFile << word << endl;
      }
    delete[] p;
    cout << "Thread number id: " << omp_get_thread_num() << " Counted: " << counterUniq << endl;
    ++it;
  } /* end for */
  cout << "Number of names with unique chars: " << uniqCount << endl;
  outputFile.close();
  return 0;
}
