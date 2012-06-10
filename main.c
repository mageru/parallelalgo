#include <stdio.h>
#include <stdlib.h>

int factorial(int n){
  if(n<2)
    return 1;
  return n*factorial(n-1);
}

int main(int argc, char *argv[]) {
  printf("%d\n",factorial(atoi(argv[1])));
}

