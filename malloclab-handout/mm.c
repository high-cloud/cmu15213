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
 * helper macros
 ********************************************************/

// Basic constants and macros
#define WSIZE 4             // word and header/footer size (bytes)
#define DSIZE 8             // double word size (bytes)
#define CHUNKSIZE 2056 //extend heap by this amount (Bytes)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

// pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// read and write a word at address p
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

// read the size and allocated fields from address p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// given block ptr bp, compute address of its header and footer
#define HDRP(bp) ((char *)(bp)-3 * WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - 2 * DSIZE)

#define PRED(bp) ((char *)(bp)-2 * WSIZE)
#define SUCC(bp) ((char *)(bp)-1 * WSIZE)

// given block ptr bp, compute address of next and previous blocks
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-2 * DSIZE))

// segregated relate
#define NUM_BLOCK 12
/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Behide Truth",
    /* First member's full name */
    "Yaodong Yang",
    /* First member's email address */
    "695397065@qq.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// global val
static size_t list_num = 8;
static size_t min_block = 5;
static unsigned int *heap_listp;

// helper function
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void place(char *bp, size_t size);
static void *find_fit(size_t size);
static void *find_list(size_t size);
static void insert_node(void *bp, size_t size);
static void delete_node(void *bp);
// static void check(void);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // create the initial empty heap
    if ((heap_listp = mem_sbrk((4 + list_num) * WSIZE)) == (void *)-1)
        return -1;
    for (int i = 0; i < list_num; i++)
    {
        PUT(heap_listp + i, 0); // header of lists
    }
    PUT(heap_listp + (list_num + 0), 0);              // for aligment
    PUT(heap_listp + (list_num + 1), PACK(DSIZE, 1)); // prologue header
    PUT(heap_listp + (list_num + 2), PACK(DSIZE, 1)); // prologue footer
    PUT(heap_listp + (list_num + 3), PACK(0, 1));     // epilogue header

    // extend the empty heap with a free block of CHUNKSIZE bytes
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t newsize = ALIGN(size + 2 * DSIZE);
    size_t extendszie;
    unsigned *bp;

    // ignore spurious requets
    if (size == 0)
        return NULL;

    // search the list for a fit
    if ((bp = find_fit(newsize)) != NULL)
    {
        place(bp, newsize);
        return bp;
    }

    // if not find fit. get more memory and place the block
    extendszie = MAX(newsize, CHUNKSIZE);

    if ((bp = extend_heap(extendszie / WSIZE)) == NULL)
        return NULL;

    delete_node(bp);
    place(bp, newsize);
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
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

static void *extend_heap(size_t words)
{
    unsigned *bp;
    size_t size;

    // change size to maintain alignment
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    bp += 2;
    // initialize free block header and footer and epilogue header
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    // coalesce if the previous block was free
    return coalesce(bp);
}

void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    // case 1
    if (prev_alloc && next_alloc)
    {
    }
    // case 2
    else if (prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_node(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    // case 3
    else if (!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_node(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else // case 4
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_node(NEXT_BLKP(bp));
        delete_node(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    insert_node(bp, size);
    return bp;
}

void *find_fit(size_t size)
{
    unsigned int *head = find_list(size);
    unsigned int *bp;
    // if no node in the list
    if (head == NULL)
        return NULL;

    while (head < heap_listp + list_num)
    {
        bp = GET(head);

        while (bp != NULL)
        {
            if (GET_SIZE(HDRP(bp)) >= size)
            {
                delete_node(bp);
                return bp;
            }
            bp = GET(SUCC(bp));
        }
        head++;
    }

    return NULL;
}

void place(char *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));
    size_t msize = 1 << min_block;
    if (csize - asize >= msize)
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        insert_node(bp, csize - asize);
    }
    else
    {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

// return ptr of list based on the size
static void *find_list(size_t size)
{
    size_t bits = 32 - __builtin_clz(size);

    if (bits <= min_block)
        return heap_listp;

    if (bits >= min_block + list_num)
        return heap_listp + (list_num - 1);

    return heap_listp + (bits - (min_block + 1));
}

void insert_node(void *bp, size_t size)
{
    unsigned *head = find_list(size);
    unsigned *old = GET(head);

    if (old != NULL)
        PUT(PRED(old), bp);
    PUT(SUCC(bp), old);
    PUT(PRED(bp), head);
    PUT(head, bp);
}
void delete_node(void *bp)
{
    unsigned *pre = GET(PRED(bp));
    unsigned *suc = GET(SUCC(bp));

    // if pre is head
    if (pre < heap_listp + list_num)
    {
        PUT(pre, suc);
    }
    else
    {
        PUT(SUCC(pre), suc);
    }

    // if suc is NULL
    if (suc != 0)
        PUT(PRED(suc), pre);
}