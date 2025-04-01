#include "csapp.h"
#include "cache.h"

static cache_list cache;
static sem_t mutex, w;
static int readcnt;
int is_cached(URL *myurl, rio_t *rio_p){

    int flag = 0;
    P(&mutex);
    readcnt++;
    if (readcnt == 1) {
        P(&w);
    }
    V(&mutex);
    cache_node *head = cache.head, *ptr = head->next;
    while(ptr){
        if(!strcmp(myurl->domain,ptr->myurl.domain) && !strcmp(myurl->uri,ptr->myurl.uri)){
            rio_writen(rio_p->rio_fd, ptr->data, ptr->node_size);
            flag = 1;
            break;
        }
        ptr = ptr->next;
    }
    P(&mutex);
    readcnt--;
    if (readcnt == 0) {
        V(&w);
    }
    V(&mutex);

    if(flag){
        delete_node(ptr);
        add_node(ptr);
        return 1;
    }
    return 0;
}

void cache_init(){
    cache.size = 0;
    cache.head = Malloc(sizeof(cache_node));
    cache.head->prev = cache.head->next = NULL;
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    readcnt = 0;
}

void add_node(cache_node *ptr){
    P(&w);
    cache_node *pos = cache.head->next, *prev_pos = cache.head;
    while(ptr->node_size + cache.size >= MAX_CACHE_SIZE){
        cache.size -= ptr->node_size;
        prev_pos->next = prev_pos->next->next;
        Free(prev_pos->next);
    }
    
    while(pos){
        prev_pos = pos;
        pos = pos->next;
    }
    cache.size += ptr->node_size;
    prev_pos->next = ptr;
    ptr->next = NULL;
    V(&w);
}


void delete_node(cache_node *ptr){
    P(&w);
    cache_node *pos = cache.head->next, *prev_pos = pos;
    while(ptr != pos){
        prev_pos = pos;
        pos = pos->next;
    }
    cache.size -= ptr->node_size;
    prev_pos->next = pos->next;
    V(&w);
}