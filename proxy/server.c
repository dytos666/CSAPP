#include "csapp.h"
#include "server.h"
#include "client.h"
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

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
void server(char *port){
    int listenfd, *connfd_p;
    socklen_t cilentlen;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    listenfd = Open_listenfd(port);
    while(1){
        connfd_p = (int*)Malloc(sizeof(int));
        *connfd_p = accept(listenfd, (SA*) &clientaddr, &cilentlen);
        Pthread_create(&tid, NULL, thread, connfd_p);
    }
}

void *thread(void *vargp){
    int connd = *((int *)vargp);
    Free(vargp);
    Pthread_detach(pthread_self());
    doit(connd);
    Close(connd);
    return NULL;
}

void doit(int fd){
    rio_t rio;
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    URL myurl;
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf,"%s %s %s", method, url, version);
    if (!strcasecmp(method, "GET")){
        parse_url(&myurl, url);
        client(&myurl, &rio);
    }
}
int parse_url(URL *myurl, char *url){
    char *uri_ptr, *domain_ptr, *port_ptr;
    domain_ptr = strstr(url, "//");
    if(domain_ptr != NULL){
        domain_ptr += 2;
        port_ptr = strstr(domain_ptr, ":");
        if(port_ptr){
            *port_ptr = '\0';
            port_ptr++;
            uri_ptr = strstr(port_ptr, "/");
            strncpy(myurl->domain, domain_ptr, MAXLINE);
            strncpy(myurl->port, port_ptr, uri_ptr - port_ptr);
            myurl->port[uri_ptr - port_ptr] = '\0';
         
        }else{
            uri_ptr = strstr(domain_ptr, "/");
            strncpy(myurl->domain, domain_ptr, uri_ptr - domain_ptr);
            strcpy(myurl->port, "80");
            myurl->domain[uri_ptr - domain_ptr] = '\0';

        }
        strcpy(myurl->uri, uri_ptr); 
    }
    return 1;
}

int sent_header(rio_t *rio, char *buf, URL *myurl){
    char read_buf[MAXLINE];
    int flag = 0;
    sprintf(buf, "GET %s HTTP/1.0\r\n", myurl->uri);
    while(1){
        Rio_readlineb(rio, read_buf, MAXLINE);
        if(!strcmp(read_buf, "\r\n")){
            break;
        }
        if(strstr(read_buf, "Host:")){
            flag = 1;
        }
        if(strstr(read_buf, "Connection:")){
            continue;
        }
        if(strstr(read_buf, "Proxy-Connection:")){
            continue;
        }
        if(strstr(read_buf, "User-Agent:")){
            continue;
        }
        sprintf(buf, "%s%s", buf, read_buf);
    }
    if(!flag){
        sprintf(buf, "%sHost: %s\r\n", buf, myurl->domain);
    }
    sprintf(buf, "%s%s", buf, user_agent_hdr);
    sprintf(buf, "%sConnect: close\r\n", buf);
    sprintf(buf, "%sProxy-Connection: close\r\n", buf);
    sprintf(buf, "%s\r\n", buf);
    return 1;
}