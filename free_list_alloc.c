#include "includes/free_list_alloc.h"
#include <stdio.h>
block_t *root_block;
block_t *tree_root;

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
    assert(root_block != NULL);
    tree_root = root_block;
}

block_t **find_free_tree(block_t **root, block_t *target)
{
    block_t **node_ptr = root;
    while (*node_ptr != target && *node_ptr != NULL) {
        if (target->size > (*node_ptr)->size)
            node_ptr = &(*node_ptr)->r;
        else
            node_ptr = &(*node_ptr)->l;
    }
    return node_ptr;
}

void insert_free_tree(block_t **root, block_t *target)
{
    block_t **node_ptr = find_free_tree(root, target);
    *node_ptr = target;
}


/*
 * Structure representing a free memory block in the memory allocator.
 * The free tree is a binary search tree that organizes free blocks (of type block_t)
 * to efficiently locate a block of appropriate size during memory allocation.
 */
void remove_free_tree(block_t **root, block_t *target)
{
    /* Locate the pointer to the target node in the tree. */
    block_t **node_ptr = find_free_tree(root, target);

    /* If the target node has two children, we need to find a replacement. */
    if ((*node_ptr)->l && (*node_ptr)->r) {
        /* Find the in-order predecessor:
         * This is the rightmost node in the left subtree.
         */
        block_t **pred_ptr = &(*node_ptr)->l;
        while ((*pred_ptr)->r)
            pred_ptr = &(*pred_ptr)->r;

        /* If the predecessor is the immediate left child. */
        if (*pred_ptr == (*node_ptr)->l) {
            block_t *old_right = (*node_ptr)->r;
            *node_ptr = *pred_ptr; /* Replace target with its left child. */
            (*node_ptr)->r = old_right; /* Attach the original right subtree. */
            assert(*node_ptr != (*node_ptr)->l);
            assert(*node_ptr != (*node_ptr)->r);
        } else {
            /* The predecessor is deeper in the left subtree. */
            block_t *old_left = (*node_ptr)->l;
            block_t *old_right = (*node_ptr)->r;
            block_t *pred_node = *pred_ptr;
            /* Remove the predecessor from its original location. */
            remove_free_tree(&old_left, *pred_ptr);
            /* Replace the target node with the predecessor. */
            *node_ptr = pred_node;
            (*node_ptr)->l = old_left;
            (*node_ptr)->r = old_right;
            assert(*node_ptr != (*node_ptr)->l);
            assert(*node_ptr != (*node_ptr)->r);
        }
    }
    /* If the target node has one child (or none), simply splice it out. */
    else if ((*node_ptr)->l || (*node_ptr)->r) {
        block_t *child = ((*node_ptr)->l) ? (*node_ptr)->l : (*node_ptr)->r;
        *node_ptr = child;
    } else {
        /* No children: remove the node. */
        *node_ptr = NULL;
    }

    /* Clear the removed node's child pointers to avoid dangling references. */
    target->l = NULL;
    target->r = NULL;
}

block_t *find_block_by_size(block_t *root, size_t size) 
{
    block_t *find = root;
    while (find != NULL && find->size != size) {
        if (size > find->size)
            find = find->r;
        else {
            if (find->l == NULL)
                break;
            find = find->l;
        }
    }
    return find;
}

void init_ptr(block_t **ptr, size_t size) 
{
    (*ptr)->size = size;
    (*ptr)->l = (*ptr)->r = (*ptr)->next = (*ptr)->prev = NULL;
}

void *fl_alloc(size_t size) 
{
    block_t *find = find_block_by_size(tree_root, size);
    if (!find)
        return NULL;
    remove_free_tree(&tree_root, find);
    if (find->size - size > sizeof(block_t)) {
        block_t *unuse = (block_t *) ((char *)(find + 1) + size);
        init_ptr(&unuse, find->size - size - sizeof(block_t));
        find->size = size;
        insert_free_tree(&tree_root, unuse);
    }

    return (char *)(find + 1);
}

void fl_free(void *ptr)
{
    if ((char *) ptr == NULL) 
        return;
    block_t *block_ptr = (block_t *) ((char *) ptr - sizeof(block_t));
    insert_free_tree(&tree_root, block_ptr);
}