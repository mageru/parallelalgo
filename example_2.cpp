#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <sys/stat.h>
#define CHUNK 4096*4
typedef struct {
  FILE *f;
  int n;
} Work;

void *p(void *arg) {
  Work *p = (Work *)arg;
  int inID=0,cnt=0;
  while(!feof(p->f)) {
    char ch = fgetc(p->f);
    if(ch == EOF) break;
    p->n--;
    if(inID) {
      if(p->n <= 0) break;
      if (isalpha(ch) || isdigit(ch)||ch=='_')
        continue;
      inID=0;
    }
    else if (isalpha(ch)) {
      inID=1, cnt++;
    }
    if (p->n <= 0) break;
  } //while
  fclose(p->f);
  p->n=cnt;
  pthread_exit((void*)0);
}

int main (int argc, char *argv[]) {
  FILE *f; int count=0,j,k;
  pthread_t x[50];
  Work work[50];
  struct stat st;
  for(int i=1;i<argc;i++) {
    printf("%s ", argv[i]);
    f=fopen(argv[i],"r");
    if (f == NULL) {
      printf("\n");
      continue;
    }
    j = fstat(fileno(f),&st);
    assert(j==0);
    printf("%d",st.st_size);
    fclose(f);
    j=0;k=0;
    while (k<st.st_size) {
      f = fopen(argv[i], "r");
      fseek(f,k, SEEK_SET);
      work[j].f = f;
      work[j].n = CHUNK;
      k += CHUNK;
      if(k >= st.st_size)
        work[j].n = st.st_size-(k-CHUNK);
      printf("k,n = %d %d \n",k,work[j].n);
      assert(
          pthread_create(&x[j],NULL, p, (void *)&work[j]) == 0);
      j++;
    } // while
    while (--j >= 0) {
      assert(pthread_join(x[j], NULL) == 0);
      printf("%d\n", work[j].n);
      count +=work[j].n;
    }
  } //for
  printf("%d\n",count);
  return count;
}

