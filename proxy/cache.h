
#ifndef __CACHE_H__
#define __CACHE_H__

#include "csapp.h"
#include "client.h"
#include "server.h"
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

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
typedef struct cache_node cache_node;
struct cache_node{
    size_t node_size;
    URL myurl;
    cache_node* prev, *next;
    char data[MAX_OBJECT_SIZE];
};


typedef struct{
    size_t size;
    cache_node *head;
}cache_list;

int is_cached(URL *myurl, rio_t *rio);
void cache_init();
void add_node(cache_node *ptr);
void delete_node(cache_node *ptr);


#endif
