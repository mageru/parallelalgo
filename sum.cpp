#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
  int count;
  int sum = 0;
  int size = argc;

  if(argc<2) {
    printf("You must input atleast two numbers to sum\n");
    exit(1);
  }

  for ( count = 1; count<argc;count++) {
    int value = atoi(argv[count]);
    sum += atoi(argv[count]);
  }

  printf("Count: %d\n", sum);
}

