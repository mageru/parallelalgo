#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

void *p(void *arg) {
  int i;
  for (i=0; i<5; i++) { printf("X\n");   sleep(1); }
  pthread_exit((void *)99);
}

int main() { //X Y interleaving is unpredictable
  pthread_t x;      void *r;     int i;
  assert(pthread_create(&x, NULL, p, (void *)34) == 0);
  for (i=0; i<5; i++) { printf("Y\n");  sleep(1); }
  assert(pthread_join(x, &r) == 0);
  return 0;
}

