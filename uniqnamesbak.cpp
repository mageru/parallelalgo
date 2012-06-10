#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

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

  int size = (int)file_contents.size();
  int *uniqCounterArray = new int[size];

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
  return 0;
}
        
    //std::cout << word << "\n";
    //std::cout << *it << "\n";
  //}
  //std::cout << "Size is: " << size << endl;
//}
/**
  if (fp == NULL) {
    printf("Error no file found!\n");
  }
  else {
    char *p;
    int k;
    while(!feof(fp)) {
      counter++;
      fscanf(fp,"%s%*[^\n]",word);
      int wordLength = strlen(word);
      int origLength = strlen(word);
      int containsDupe = 0;
      p=word;

      for(int i=0;i<wordLength; i++) {
        for(int j=0;j<wordLength; j++) {
          if(i==j) {
            continue;
          }
          else if(* (p+i)==*(p+j)) {
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
      //printf("word read is: %s Dupe: %d Size: %d\n", word, containsDupe,origLength);
      if(containsDupe == 0 && origLength >= 6) {
        printf("Line: %d Word: %s Dupe: %d Size: %d\n",counter, word, containsDupe,origLength);
      }
      strcpy(word,"");
    }
  }
  fclose(fp);
}
**/
  /**
  FILE *fp;
  string word[30];
  int counter=0;
  string line;
  
  //fp = fopen("/home/milleju/dev/para/dist.female.first","r");
  ifstream namesFile ("/home/milleju/dev/para/dist.female.first");
  if (namesFile.is_open())
  {
    while(! namesFile.eof() )
    {
      getline (namesFile,line);
      word[counter] = line;
      cout << word[counter] << endl;
      counter++;
    }
    namesFile.close();
  }
  else cout << "Unable to open file";
  system("PAUSE");
  return 0;
}
**/


