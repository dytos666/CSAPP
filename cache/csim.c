#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

typedef struct{
    int miss;
    int hit;
    int eviction;
}info_count;

typedef struct{
    int s;
    int E;
    int b;
    char* filepath;
}cache_bit_format;

typedef struct{
    int valid;
    unsigned tag;
    unsigned time_stamp;
}cache_line;

typedef struct{
    cache_line** line;
}cache_set;

typedef struct{
    cache_set** set;   
}cache_table;

cache_table* cache_init();
void cache_modify(char operation, unsigned address, cache_table* table, info_count* p_c, cache_bit_format *f);
void clear(info_count *p_c, cache_bit_format *f, cache_table*table);
void get_format(cache_bit_format *f,int argc, char **argv){
    int opt;
    while(-1 != (opt = getopt(argc, argv, "s:E:b:t:"))){
        switch(opt){
            case 's':
                f->s = atoi(optarg);
                break;
            case 'E':
                f->E = atoi(optarg);
                break;
            case 'b':
                f->b = atoi(optarg);
                break;
            case 't':
                f->filepath = optarg;
                break;
            default :
                break;
        }
    }
}

info_count* info_count_init(){
    info_count* p_c = (info_count*) malloc(sizeof(info_count));
    if(p_c){
        p_c->miss = 0;
        p_c->hit = 0;
        p_c->eviction = 0;
        return p_c;
    }
    return NULL;
}
void read_file_info(char* filepath, cache_table* table, cache_bit_format *f, info_count *p_c){
    FILE* file;
    file = fopen(filepath, "r");
    char operation;
    unsigned address;
    int size;
    while(fscanf(file, " %c %x,%d\n", &operation, &address, &size) > 0){
        //printf("%c %x,%d\n",operation,address,size);
        cache_modify(operation, address, table, p_c, f);
    }
    fclose(file);
}
int main(int argc, char **argv)
{
    info_count* p_c = info_count_init();

    cache_bit_format *f = (cache_bit_format *) malloc(sizeof(cache_bit_format));
    get_format(f, argc, argv);

    cache_table* table = cache_init(f);

    read_file_info(f->filepath, table, f, p_c);
    printSummary(p_c->hit, p_c->miss, p_c->eviction);
    return 0;
}



cache_table* cache_init(const cache_bit_format* f){
    cache_table* table = (cache_table*) malloc(sizeof(cache_table));
    table->set = (cache_set**) malloc(sizeof(cache_set*)*(1 << f->s));
    for(unsigned i = 0; i < (1 << f->s); i++){
        table->set[i] = (cache_set*) malloc(sizeof(cache_set));
        table->set[i]->line = (cache_line**) malloc(sizeof(cache_line*));
        for(int j = 0; j < f->E; j++){
            table->set[i]->line[j] = (cache_line*) malloc(sizeof(cache_line));
            table->set[i]->line[j]->valid = 0;
            table->set[i]->line[j]->tag = 0;
            table->set[i]->line[j]->time_stamp = 0;
        }
    }
    return table;
}

void cache_modify(char operation, unsigned address, cache_table* table, info_count* p_c, cache_bit_format* f){
    if(operation == 'I')return;
    unsigned tag, set;
    set = (address >> f->b) % (1 << f->s);
    tag = (address >> (f->b + f->s));
    if(operation == 'M'){
        p_c->hit++;
    }
    int i,signal = 0;
    for(i = 0; i < f->E; i++){
        if(table->set[set]->line[i]->time_stamp == 0 && signal == 1) return;
        if(table->set[set]->line[i]->time_stamp == 0){
            table->set[set]->line[i]->tag = tag;
            table->set[set]->line[i]->valid = 1;
            table->set[set]->line[i]->time_stamp++;
            p_c->miss++;
            return;
        }else if(table->set[set]->line[i]->tag == tag && table->set[set]->line[i]->valid == 1){
            table->set[set]->line[i]->time_stamp = 0;
            p_c->hit++;
            signal = 1;
        }
        table->set[set]->line[i]->time_stamp++;
    }
    if(signal == 1){
        return;
    }
    int pos = 0;
    for(int i = 1; i < f->E; i++){
        if(table->set[set]->line[pos]->time_stamp < table->set[set]->line[i]->time_stamp){
            pos = i;
        }
    }
    table->set[set]->line[pos]->time_stamp = 1;
    table->set[set]->line[pos]->tag = tag;
    p_c->eviction++;
    p_c->miss++;


}
void clear(info_count *p_c, cache_bit_format *f, cache_table *table){
    for(int i = 0; i < (1 << f->s);i++){
        for(int j = 0; j < f->E; j++){
            free(table->set[i]->line[j]);
        }
        free(table->set[i]);
    }
    free(table);
    free(f);
    free(p_c);
}