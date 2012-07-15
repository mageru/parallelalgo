#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
/* Run the code until it stops at the accept. Then start a web browser and enter http://localhost/index.html The program should print the content of the HTTP (hypertext transfer protocol) GET packet. If port 80 is already in use or protected, use public port 1080 in the program and try telnet localhost 1080.  The program should output the first line entered. */
int main(int argc, char *argv[]) {
    int i; char str[1024]; int s, news;
    unsigned int j;
    struct sockaddr_in si;
    struct hostent *h;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    memset(&si, 0, sizeof(si));
    h = gethostbyname("localhost");
    memcpy(&si.sin_addr, h->h_addr_list[0], sizeof(struct in_addr));
    si.sin_port = htons(80);
    si.sin_family = AF_INET;
    i = bind(s, (const struct sockaddr *)&si, sizeof(si));
    listen(s, 5);
    j = sizeof(si);
    news = accept(s, (struct sockaddr *)&si, &j);
    if (news > 0) {
        i = recv(news, str, sizeof(str), 0);
        str[i] = 0; /* C/C++ strings end with '\0' */
        printf("%s\n", str);
        close(news);
    }
    close(s);
}
