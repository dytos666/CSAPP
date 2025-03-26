/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "6666666666666666666666",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

#define MIN_SIZE 16

#define SEGREGATED_NUM 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))
#define MIN(x, y) ((x) < (y)? (x) : (y))
/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) (GET(p) & 0x2)


/* Given block ptr bp,
compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


#define GET_PREV_POS(bp)  ((char*)mem_heap_lo() + *(unsigned*)(bp))
#define GET_SUCC_POS(bp)  ((char*)mem_heap_lo() + *(unsigned*)((char*)(bp) + WSIZE))
#define SET_PREV_POS(bp, val) (*(unsigned*)(bp) = ((unsigned)(long)(val) - (unsigned)(long)mem_heap_lo()))
#define SET_SUCC_POS(bp, val) (*(unsigned*)((char*)(bp) + WSIZE) = ((unsigned)(long)(val) - (unsigned)(long)(mem_heap_lo())))
//used variables
static char *heap_listp = 0;

//helpr func
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static size_t max(size_t x, size_t y);
static int class_fit(size_t size);
static void place(void *bp, size_t asize);
static void delete_free_block(void* bp);
static void add_free_block(void*, size_t);
static void *coalesce(void *bp);
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp = mem_sbrk((4 + 2*SEGREGATED_NUM) * WSIZE)) == (void*)-1)
        return -1;
    PUT(heap_listp, 0);
    heap_listp += WSIZE;
    //initialize the segregated pointer
    for(size_t i = 1; i <= SEGREGATED_NUM; i++){
        PUT(heap_listp + (i * DSIZE), 0);
        PUT(heap_listp + (i * DSIZE) + WSIZE, 0);
    }

    PUT(heap_listp + (    2*SEGREGATED_NUM) * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + (1 + 2*SEGREGATED_NUM) * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + (2 + 2*SEGREGATED_NUM) * WSIZE, PACK(0, 3));


    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if(heap_listp == 0) mm_init();
    if(size == 0) return NULL;
    char *bp;
    size_t extendsize;
    size_t newsize = DSIZE * ((size + (WSIZE)+(DSIZE - 1)) / DSIZE);
    newsize = max(newsize, MIN_SIZE);
    int index = class_fit(newsize);

    for(int i = index; i < SEGREGATED_NUM; i++){
        void *s_p = GET_SUCC_POS(heap_listp + i * DSIZE);
        while(s_p - mem_heap_lo()){
            size_t alloc_size = GET_SIZE(HDRP(s_p));
            
            if(alloc_size >= newsize){
                place(s_p, newsize);
                return s_p;
            }
            s_p = GET_SUCC_POS(s_p);
        }

    }
    extendsize = max(newsize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
            return NULL;
    place(bp, newsize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if(ptr == NULL) 
        return;
    size_t size = GET_SIZE(HDRP(ptr));
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, prev_alloc));
    PUT(FTRP(ptr), PACK(size, prev_alloc));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void* newptr;
    if (size == 0) {
        mm_free(ptr);
        return 0;
    }
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    newptr = mm_malloc(size);
    if (!newptr) {
        return 0;
    }
    oldsize = GET_SIZE(HDRP(ptr));
    oldsize = MIN(oldsize, size);
    memcpy(newptr, ptr, oldsize);
    mm_free(ptr);

    return newptr;
}




static void *extend_heap(size_t words){
    char *bp;
    size_t size;
    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
    return NULL;

    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, prev_alloc)); /* Free block header */
    PUT(FTRP(bp), PACK(size, prev_alloc)); /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
 }

static size_t max(size_t x, size_t y){
    return x > y ? x : y;
}

static int class_fit(size_t size){
    if(size == 0) return 0;
    int index = 0;
    while(size){
        index++;
        size >>= 1;
    }
    index -= 4;
    if(index >= SEGREGATED_NUM) index = SEGREGATED_NUM - 1;
    return index;
    
}

static void place(void *bp, size_t asize){
    size_t cszie = GET_SIZE(HDRP(bp));
    size_t prev_alloc = GET_PREV_ALLOC((HDRP(bp))); 
    delete_free_block(bp);
    if((cszie - asize) >= MIN_SIZE){
        PUT(HDRP(bp), PACK(asize, GET_PREV_ALLOC(HDRP(bp)) + 1));


        void *n_bp = (void *)NEXT_BLKP(bp);
        PUT(HDRP(n_bp), PACK((cszie - asize), 2));
        PUT(FTRP(n_bp), PACK((cszie - asize), 2));

        add_free_block(n_bp, cszie - asize);
        
    }else{
        PUT(HDRP(bp), PACK(cszie, prev_alloc + 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(GET(HDRP(NEXT_BLKP(bp))), 2));
    }
    
}

static void delete_free_block(void *bp){
    char* prev = GET_PREV_POS(bp);
    char* next = GET_SUCC_POS(bp);
    if(next != mem_heap_lo()){
        SET_PREV_POS(next, prev);
        SET_SUCC_POS(prev, next);
    }else{
        SET_SUCC_POS(prev, next);
    }
}

static void add_free_block(void *n_bp, size_t size){

    int index = class_fit(size); 
    char* next = GET_SUCC_POS(heap_listp + DSIZE * index);
    SET_PREV_POS(n_bp, heap_listp + DSIZE * index);
    SET_SUCC_POS(heap_listp + DSIZE * index, n_bp);
    SET_SUCC_POS(n_bp, next);
    if(next != mem_heap_lo()){
        SET_PREV_POS(next, n_bp);
    }
    return;
}




static void *coalesce(void *bp){
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    char *next_head = HDRP(NEXT_BLKP(bp));
    if(prev_alloc && next_alloc){
        PUT(next_head, PACK(GET_SIZE(next_head), 1));
        add_free_block(bp, size);
        return bp;

    }else if(prev_alloc && !next_alloc){
        delete_free_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 2));
        PUT(FTRP(bp), PACK(size, 2));
        add_free_block(bp, size);

    }else if(!prev_alloc && next_alloc){
        delete_free_block(PREV_BLKP(bp));
        PUT(next_head, PACK(GET_SIZE(next_head), 1));
        void *prev_p = PREV_BLKP(bp);
        size += GET_SIZE(HDRP(prev_p));
        PUT(HDRP(prev_p), PACK(size, GET_PREV_ALLOC(HDRP(prev_p))));
        PUT(FTRP(prev_p), PACK(size, GET_PREV_ALLOC(HDRP(prev_p))));
        add_free_block(prev_p, size);
        return prev_p;

    }else{
        void *prev_p = PREV_BLKP(bp);
        delete_free_block(prev_p);
        delete_free_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(prev_p)) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(prev_p), PACK(size, GET_PREV_ALLOC(HDRP(prev_p))));
        PUT(FTRP(prev_p), PACK(size, GET_PREV_ALLOC(HDRP(prev_p))));
        add_free_block(prev_p, size);
        return prev_p;
    }
    return NULL;
}