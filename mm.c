/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this approach, we used an implicit free list. In each block, there is a header, then a "size" number of bytes allocated for the payload, followed by a footer. Coalesce is used to combine free blocks in succession. When free is called on a block, the allocated bit in the block's header and footer changes to zero. Realloc reallocates space in the heap.
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
    "luv2code",
    /* First member's full name */
    "Samuel Arrants",
    /* First member's email address */
    "samarrants@u.northwestern.edu",
    /* Second member's full name (leave blank if none) */
    "Lindsay Wilson",
    /* Second member's email address (leave blank if none) */
    "lindsaywilson2020@u.northwestern.edu"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define WSIZE 4 /* Word and header/footer size (bytes) */
#define DSIZE 8 /* double word size (bytes) */
#define CHUNKSIZE (1<<12) /* extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x): (y))

/* Combine size and alloc together */
#define PACK(size, alloc) ((size | alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block pointer bp, find address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block pointer bp, find address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* getting size of block header */
#define GET_BLOCKHDR(bp) ((char *)(bp) - WSIZE)
#define GET_CURR_SIZE(bp) (GET_SIZE(GET_BLOCKHDR(bp)))

//helper functions
static char *heap_listp;
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/* Coalesce combines blocks together if both free after calling mm_free */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    //Case 1: if neither the preceding or succeeding blocks are free
    if (prev_alloc && next_alloc){
        return bp;
    }
    
    //Case 2: if the succeeding block is free
    else if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    
    //Case 3: if preceding block is free
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    
    //Case 4: if both preceding and succeeding blocks are free
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/* extend_heap extends the size of the heap when there is no appropriate block for allocating */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    
    // Allocate an even number of words to maintain alignment
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    // Initialize free block header/footer and the epilogue header
    PUT(HDRP(bp), PACK(size, 0)); //free block header
    PUT(FTRP(bp), PACK(size, 0)); //free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //new epilogue header
    
    //Coalesce if the previous block was free
    return coalesce(bp);
}
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the intitial empty heap */
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                             // Alignment padding
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    // prologue header
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    // prologue footer
    PUT(heap_listp + (3*WSIZE), PACK(0,1));         // epilogue header
    heap_listp += (2*WSIZE);
    // if error, return -1//
    // extend empty heap with a free block of CHUNKSIZE bytes
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}



/* find_fit uses a first-fit search approach to determine the first block that can be allocated */
static void *find_fit(size_t asize)
{
    // first-fit search
    void *bp;
    
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }
    return NULL; //no fit
}

/* place allocates block by setting size and availabilty in header and footer */
static void place(void *bp, size_t asize){
    size_t csize = GET_SIZE(HDRP(bp));
    
    if ((csize - asize) >= (2*DSIZE)){
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
    }
    else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}
/*
 * mm_malloc - Allocate a block by traversing the heap until an appropriate block is found. If no appropriate block is available, the heap is extended. It allocates by setting the header and footer.
 */
void *mm_malloc(size_t size)
{
    size_t asize; // adjusted block size
    size_t extendsize; //amount to extend heap if no fit
    char *bp;
    
    //ignore spurious requests
    if(size == 0){
        return NULL;
    }
    
    // adjust block size to include overhead and alignment reqs
    if (size <= DSIZE){
        asize = 2*DSIZE;
    }
    else{
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    }
    
    //search the free list for a fit
    if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }
    
    // No fit found. Get more memory and place the block
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL){
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Changes the allocated bit in the header and footer to zero and then coalesces.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL){
        return NULL;
    }
    copySize = GET_CURR_SIZE(oldptr);
    
    if (size < copySize){
        copySize = size;
    }
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}


int mm_check(void){
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        /* if header and footer don't match */
        if(GET_ALLOC(HDRP(bp)) != GET_ALLOC(FTRP(bp))){
            return 0;
        }
        /* if blocks escaped coalescing */
        else if(GET_ALLOC(bp) == 0 && GET_ALLOC(NEXT_BLKP(bp)) == 0){
            return 0;
        }
        /* if blocks are overlapping */
        else if(FTRP(bp) >= NEXT_BLKP(bp)){
            return 0;
        }
    }
    
    return 1;
}










