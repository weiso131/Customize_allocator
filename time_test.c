#include "includes/free_list_alloc.h"
#include <time.h>
#include <stdio.h>

#define TEST_TIMES 1000000
#define MAX_POINTERS 1000000
#define MAX_ALLOC_SIZE 1000


void time_test(void *(*alloc)(size_t), void (*free_)(void *), char *filename)
{

    FILE *file = fopen(filename, "a"); // 以附加模式 (append) 開啟檔案
    if (file == NULL) {
        perror("無法開啟檔案");
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    void *ptrs[MAX_POINTERS] = {0};

    clock_t total_alloc_time = 0, total_free_time = 0;

    for (int i = 0; i < TEST_TIMES; ++i) {
        int op = rand() % 2;

        if (op == 1) {
            size_t size = rand() % (MAX_ALLOC_SIZE + 1);
            clock_t start = clock();
            void *p = alloc(size);
            clock_t end = clock();
            total_alloc_time += (end - start);

            ptrs[i] = p;
            fprintf(file, "%ld\n", end - start);
        } else {
            int index = rand() % (i + 1);
            clock_t start = clock();
            free_(ptrs[index]);
            clock_t end = clock();
            ptrs[index] = NULL; // avoid double free
            total_free_time += (end - start);
            fprintf(file, "%ld\n", end - start);
        }
    }
    

    printf("total_time: %ld\n", total_alloc_time + total_free_time);
    fclose(file);
}

int main() 
{
    init_fl(1 << 30);
    printf("free list\n");
    time_test(fl_alloc, fl_free, "free_list.txt");
    free(root_block);
    free(root_head);
    printf("malloc\n");
    time_test(malloc, free, "malloc.txt");
}