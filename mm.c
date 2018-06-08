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


#define SIZE_T_SIZE (ALIGN(sizeof(size_t))
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


/* Read the previous and next of the node */
#define GET_PREV(p) (GET(p + (1*WSIZE)) //this should be the address of prev node
#define GET_NEXT(p) (GET(p + (2*WSIZE)) //this should be the address of next node


/* Given block pointer bp, find address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block poitner bp, find address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
/* 
 * mm_init - initialize the malloc package.
 */
//helper functions
static char *heap_listp;
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void removenode(void *bp)
static void addnode(void *bp)


static void removenode(void *bp){
/*
    currnode.prev.next = currnode.next
    currnode.next.prev = currnode. prev*/
}



static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    //Case 1
    if (prev_alloc && next_alloc){
        return bp;
    }
    
    //Case 2
    else if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    
    //Case 3
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    
    //Case 4
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}
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
int mm_init(void)
{
    /* Create the intitial empty heap */
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                             // Alignment padding
    //below, we want the prologue to function like a free block.
    // prologue.next should be address to first free block. still allowed a header and footer.
    //Why is there padding in the inital version of this?
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



/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
static void *find_fit(size_t asize)
{   // this is where we go from address to address
    // first-fit search
    void *bp;
    
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }
    return NULL; //no fit
}

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
void *mm_malloc(size_t size)
{
    size_t asize; // adjusted block size
    size_t extendsize; //amount to extend heap if no fit
    char *bp;
    
    //ignore spurious requests
    if(size == 0)
        return NULL;
    
    // adjust block size to include overhead and alignment reqs
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    
    
    
    //search the free list for a fit
    
    //for explicit list, we want to jump from prologue -> free block -> free block
    /* Sam sudo code
    currnode = prologue.next
     if currnode.size < asize {
        currnode = currnode.next}
     if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
         // currnode.prev.next = currnode.next
         // currnode.next.prev = currnode.prev
        return bp;
     }*/
     
    
    //search the free list for a fit
    if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }
    
    // No fit found. Get more memory and place the block
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
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
    if(ptr == NULL){
        mm_malloc(size);
    }
    if(size == 0){
        mm_free(ptr);
    }
    else{
        return ptr;
    } 
    /*void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;*/
}














