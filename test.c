#include "includes/free_list_alloc.h"
#include <time.h>
#include <stdio.h>

#define TEST_TIMES 10000
#define MAX_POINTERS 10000
#define MAX_ALLOC_SIZE 100000

int list_size()
{
    int count = 0;
    struct list_head *node = NULL;
    list_for_each(node, root_head) {
        count++;
    }
    return count;
}

void show()
{
    struct list_head *node = NULL;
    list_for_each(node, root_head) {
        printf("%ld ", list_entry(node, block_t, list)->size);
    }
    printf("\n");
}
void show_use()
{
    struct list_head *node = NULL;
    list_for_each(node, root_head) {
        printf("%d ", list_entry(node, block_t, list)->use);
    }
    printf("\n");
}

void stress_test()
{
    srand(time(NULL));
    void *ptrs[MAX_POINTERS] = {0}; 
    for (int i = 0; i < TEST_TIMES; ++i) {
        int op = rand() % 2;  // 0 = alloc, 1 = free

        if (op == 0) {

            size_t size = rand() % (MAX_ALLOC_SIZE + 1);
            //printf("op: %d, size: %ld\n", op, size);
            void *p = fl_alloc(size);
            ptrs[i] = p;  
        } else {
            int index = rand() % (i + 1); 
            //printf("op: %d, ptr: %p\n", op, ptrs[index]);
            fl_free(ptrs[index]); 
            ptrs[index] = NULL;
        }
    }
}

int main()
{
    int count = 0;
    init_fl(1 << 30);

    int *array = (int *)fl_alloc(30 * sizeof(int));
    int *array2 = (int *)fl_alloc(30 * sizeof(int));   
    assert((char *)array2 - (char *)array - sizeof(block_t) == 120);
    printf("array2 is located right after the memory space allocated for array1\n");

    fl_free(array);
    int *array3 = (int *)fl_alloc(30 * sizeof(int));
    assert(array3 == array);
    printf("Reused freed block successfully (array3 == array1)\n");

    count = list_size();
    assert(count == 3);
    printf("All blocks are present in the linked list\n");

    fl_free(array2);
    fl_free(array3);
    count = list_size();
    assert(count == 1);
    printf("Block merging confirmed\n");

    show();
    int *array4 = (int *)fl_alloc(50 * sizeof(int));
    count = list_size();
    assert(count == 2);
    printf("Allocator still works correctly after merging\n");

    fl_free(array4);
    array = (int *)fl_alloc(30 * sizeof(int));
    assert(count == 2);
    
    fl_free(array);
    assert(tree_root->size == 1 << 30);

    array = (int *)fl_alloc(30 * sizeof(int));
    array2 = (int *)fl_alloc(30 * sizeof(int));
    fl_free(array);
    array3 = (int *)fl_alloc(40 * sizeof(int));
    count = list_size();
    assert(count == 4);

    fl_free(array2);
    show();
    fl_free(array3);
    count = list_size();
    assert(count == 1);

    array = fl_alloc(0);
    assert(array == NULL);

    
    
    printf("start stress test\n");
    stress_test();
    printf("ALL TEST COMPLETE!!!\n");
}





