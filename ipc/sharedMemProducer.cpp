#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SIZE 8192
int main(int argc, char *argv[]) {
int shmid;
char * adr;
shmid = shmget((key_t)1234, SIZE, 0600|IPC_CREAT);
if (shmid == -1) {
  perror("producer"); exit(EXIT_FAILURE);
}
adr = shmat(shmid, NULL, 0);
if (adr == (char *)-1) {
  perror("producer"); exit(EXIT_FAILURE);
}
printf("shared memory at %x\n", (int)adr);
strcpy(adr, "hello from another process\n");
if (shmdt(adr) == -1) {
  perror("producer"); exit(EXIT_FAILURE);
}
exit(EXIT_SUCCESS);
}
