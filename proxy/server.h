#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "csapp.h"


typedef struct {
    char domain[MAXLINE];
    char uri[MAXLINE];
    char port[MAXLINE];
}URL;

void server(char *port);
void doit(int fd);
void *thread(void *vargp);
int parse_url(URL *myurl, char *url);
int sent_header(rio_t *rio, char *buf, URL *myurl);



#endif
