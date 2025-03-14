#include "includes/free_list_alloc.h"
#include <stdio.h>
block_t *root_block;
block_t *tree_root;
struct list_head *root_head;

block_t *init_block(size_t size)
{
    block_t *block = malloc(sizeof(block_t) + size);
    block->size = size;
    block->l = block->r = NULL;
    block->use = 0;
    return block;
}

void init_fl(size_t size)
{
    root_block = init_block(size);
    assert(root_block != NULL);
    tree_root = root_block;
    root_block->pprev = &tree_root;
    root_head = malloc(sizeof(struct list_head));
    INIT_LIST_HEAD(root_head);
    list_add(&root_block->list, root_head);
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
    (*node_ptr)->pprev = node_ptr;
}


/*
 * Structure representing a free memory block in the memory allocator.
 * The free tree is a binary search tree that organizes free blocks (of type block_t)
 * to efficiently locate a block of appropriate size during memory allocation.
 */
void remove_free_tree(block_t **root, block_t *target)
{
    /* Locate the pointer to the target node in the tree. */
    block_t *pred_ptr = NULL;

    /* If the target node has two children, we need to find a replacement. */
    if (target->l && target->r) {
        /* Find the in-order predecessor:
         * This is the rightmost node in the left subtree.
         */
        pred_ptr = target->l;
        while (pred_ptr->r)
            pred_ptr = pred_ptr->r;

        /* If the predecessor is the immediate left child. */
        if (pred_ptr == target->l) {
            block_t *old_right = target->r;
            pred_ptr->r = old_right; /* Attach the original right subtree. */
            old_right->pprev = &pred_ptr->r;
        } else {
            /* The predecessor is deeper in the left subtree. */
            block_t *old_left = target->l;
            block_t *old_right = target->r;
            block_t *pred_node = pred_ptr;
            /* Remove the predecessor from its original location. */
            remove_free_tree(&old_left, pred_ptr);
            /* Replace the target node with the predecessor. */
            pred_ptr->l = old_left;
            old_left->pprev = &pred_ptr->l;
            pred_ptr->r = old_right;
            old_right->pprev = &pred_ptr->r;
        }
    }
    /* If the target node has one child (or none), simply splice it out. */
    else if (target->l || target->r) {
        block_t *child = (target->l) ? target->l : target->r;
        pred_ptr = child;
    } else {
        /* No children: remove the node. */
        pred_ptr = NULL;
    }
    *(target->pprev) = pred_ptr;
    if (pred_ptr)
        pred_ptr->pprev = target->pprev;
    /* Clear the removed node's child pointers to avoid dangling references. */
    target->pprev = NULL;
    target->l = NULL;
    target->r = NULL;
}

block_t *find_block_by_size(block_t *root, size_t size) 
{
    block_t *find = root, *last_bigger = NULL;
    while (find != NULL && find->size != size) {
        if (size > find->size)
            find = find->r;
        else {
            last_bigger = find;
            find = find->l;
        }
    }
    if (!find)
        find = last_bigger;
    return find;
}

void init_ptr(block_t **ptr, size_t size) 
{
    (*ptr)->size = size;
    (*ptr)->l = (*ptr)->r = NULL;
    (*ptr)->use = 0;
}

void *fl_alloc(size_t size) 
{
    if (size == 0)
        return NULL;
    block_t *find = find_block_by_size(tree_root, size);
    if (!find)
        return NULL;
    remove_free_tree(&tree_root, find);
    if (find->size - size > sizeof(block_t)) {
        block_t *unuse = (block_t *) ((char *)(find + 1) + size);
        init_ptr(&unuse, find->size - size - sizeof(block_t));
        find->size = size;
        list_add(&unuse->list, &find->list);
        insert_free_tree(&tree_root, unuse);
    }
    find->use = 1;
    return (char *)(find + 1);
}

void merge_b_to_a(block_t *a, block_t *b)
{
    remove_free_tree(&tree_root, b);
    list_del(&b->list);
    remove_free_tree(&tree_root, a);
    a->size = a->size + b->size + sizeof(block_t);
    insert_free_tree(&tree_root, a);
}

int check_is_use(struct list_head *x)
{
    if (x == root_head) return 0;
    return !list_entry(x, block_t, list)->use;
}

void try_merge(block_t *block)
{
    struct list_head *next = block->list.next, *prev = block->list.prev;
    if (check_is_use(next)) 
        merge_b_to_a(block, list_entry(next, block_t, list));
    if (check_is_use(prev))
        merge_b_to_a(list_entry(prev, block_t, list), block);
}

void fl_free(void *ptr)
{
    if ((char *) ptr == NULL) 
        return;
    block_t *block_ptr = (block_t *) ((char *) ptr - sizeof(block_t));
    if (block_ptr->use == 0)
        return;
    block_ptr->use = 0;
    insert_free_tree(&tree_root, block_ptr);
    try_merge(block_ptr);
}