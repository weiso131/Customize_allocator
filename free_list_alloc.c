#include "includes/free_list_alloc.h"
block_t *init_block(size_t size)
{
    block_t *block = malloc(sizeof(block_t) + size);
    block->size = size;
    block->l = block->r = block->next = block->prev = NULL;
    return block;
}

void init_fl(size_t size)
{
    root_block = init_block(size);
}