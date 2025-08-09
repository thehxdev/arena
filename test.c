#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include "arena.h"

/* FIXME: This test is not complete! */

#ifdef __linux__
void *allocator_mmap(void *p, unsigned long size);
#endif

void *allocator_malloc(void *p, unsigned long size);

int main(void) {
    arena_t a;
    long *p, *z;

    #ifdef __linux__
    if (!arena_init(&a, 4ul * 1024, allocator_mmap, ARENA_STACK|ARENA_FIXED)) {
        perror("arena");
        return 1;
    }
    #else
    if (!arena_init(&a, 4ul * 1024, allocator_malloc, ARENA_STACK|ARENA_FIXED)) {
        perror("arena");
        return 1;
    }
    #endif

    p = arena_alloc(&a, sizeof(long));
    *p = 111;
    printf("last value size = %lu\n", arena_last_size(&a));

    z = arena_pop(&a);
    printf("value = %ld\n", *z);
    assert(p == z);

    arena_deinit(&a);
    return 0;
}

#ifdef __linux__
void *allocator_mmap(void *p, unsigned long size) {
    void *tmp = NULL;
    unsigned long psize;
    if (size == 0) {
        psize = *((unsigned long*)p - 1);
        munmap(p, psize);
        return NULL;
    }
    tmp = mmap(NULL,
               size + sizeof(psize),
               PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANON,
               -1, 0);
    if (tmp == MAP_FAILED)
        return NULL;
    *((unsigned long*)tmp) = size;
    return ((unsigned long*)tmp + 1);
}
#endif

void *allocator_malloc(void *p, unsigned long size) {
    if (size == 0) {
        free(p);
        return NULL;
    }
    return malloc(size);
}
