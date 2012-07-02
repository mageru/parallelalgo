//execute the following commands as root to enable mqs
// mkdir /dev/mqueue
// mount -t mqueue none /dev/mqueue
// ls /dev/mqueue will display active mqs
// compile with -lrt to link with real-time library
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <assert.h>

int main(int argc, char *argv[]) {
mqd_t m;
int i;
struct mq_attr mqa;
char s[24];
mqa.mq_flags=0;
mqa.mq_maxmsg=8;  //default is 10
mqa.mq_msgsize=16;//default is 8192
mqa.mq_curmsgs=0;
m=mq_open("/m_mq", O_RDONLY | O_CREAT,S_IRWXU, &mqa);
assert(m>=0);
i=mq_receive(m, s, 16, NULL);
assert(i==16);
printf("%s\n", s);
mq_close(m);
mq_unlink("/m_mq");
return 0;
}
