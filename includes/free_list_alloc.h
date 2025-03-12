#ifndef FREE_LIST_ALLOC_H
#define FREE_LIST_ALLOC_H

#include <stdlib.h>
#include <assert.h>
typedef struct block {
    size_t size;
    struct block *next, *prev;
    struct block *l, *r;
} block_t;



/**
 * init_fl - Initialize the maximum memory limit 
 * available for the free list allocator.
 * @size: max memory size
 */
void init_fl(size_t size);

/**
 * fl_alloc - Allocate memory based on the size 
 * provided by the user and return a pointer.
 * If the allocation fails, return NULL.
 * @size: size provided by the user
 */
void *fl_alloc(size_t size);

/**
 * fl_free - Free the memory corresponding to 
 * the pointer provided by the user.
 * @ptr: target memory to free
 */
void fl_free(void *ptr);

#endif