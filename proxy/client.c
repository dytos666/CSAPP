#include "client.h"
#include "csapp.h"
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
void client(URL *myurl, rio_t *rio_p){
    if(is_cached(myurl, rio_p)){
        return;
    }
    char head_buf[MAXLINE];
    char read_buf[MAXLINE];
    sent_header(rio_p, head_buf, myurl);
    int read_num;
    int size = 0;
    char *port = myurl->port;
    int clientfd = open_clientfd(myurl->domain, port);
    cache_node* mynode = Malloc(sizeof(cache_node));
    rio_t rio;
    Rio_readinitb(&rio, clientfd);
    if(rio_writen(rio.rio_fd, head_buf, strlen(head_buf)) < 0){
        close(clientfd);
        return;
    }

    while((read_num = rio_readnb(&rio, read_buf, MAXLINE)) > 0){
        if(read_num < 0){
            close(clientfd);
            return;
        }
        if(size + read_num <= MAX_OBJECT_SIZE){
            memcpy(mynode->data + size, read_buf, read_num);
        }
        size += read_num;
        if(rio_writen(rio_p->rio_fd, read_buf, read_num) < 0){
            fprintf(stderr, "Send response to client error\n");
            close(clientfd);
            return;
        }

    }
    if(size <= MAX_OBJECT_SIZE){
        mynode->node_size = size;
        strncpy(mynode->myurl.domain, myurl->domain, MAXLINE);
        mynode->myurl.domain[MAXLINE - 1] = '\0';
        strcpy(mynode->myurl.port, myurl->port);
        strcpy(mynode->myurl.uri, myurl->uri);
        add_node(mynode);
    }else{
        Free(mynode);
    }
    close(clientfd);
    return;
}