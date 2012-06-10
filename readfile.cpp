#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() 
{
  FILE *fp;
  char word[30];
  int counter=0;
  
  fp = fopen("/home/milleju/dev/para/dist.female.first","r");
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
