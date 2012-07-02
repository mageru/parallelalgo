#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <assert.h>

int main(int argc, char *argv[]) {
mqd_t m;
int i;
char s[24]="abcdefghijklmno";
m=mq_open("/m_mq", O_WRONLY);
assert(m>=0);
i=mq_send(m, s, 16, 1);
perror("W");
mq_close(m);
return 0;
}
