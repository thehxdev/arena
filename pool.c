#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include "arena.h"

/*
 * Generic memory pool allocator using arena allocator
 */

typedef union chunk chunk_t;
union chunk {
    chunk_t *next;
};

typedef struct pool {
    arena_t *arena;
    chunk_t *freelist;
    size_t  chunksize;
} pool_t;

void pool_init(pool_t *p, size_t chunksize) {
    arena_config_t aconf = ARENA_DEFAULT_CONFIG;
    chunksize = (chunksize > sizeof(chunk_t)) ? chunksize : sizeof(chunk_t);
    *p = (pool_t){
        .arena = arena_new(&aconf),
        .freelist = NULL,
        .chunksize = sizeof(long),
    };
}

void pool_destroy(pool_t *p) {
    arena_destroy(p->arena);
}

void *pool_get(pool_t *self) {
    chunk_t *c;
    if (!self->freelist)
        return arena_alloc(self->arena, self->chunksize);
    c = self->freelist;
    self->freelist = c->next;
    return c;
}

void pool_put(pool_t *self, void *v) {
    chunk_t *c = (chunk_t*) v;
    c->next = self->freelist;
    self->freelist = c;
}

int main(void) {
    pool_t p;
    pool_init(&p, sizeof(long));

    long *i = pool_get(&p);
    *i = 0xBabaAbDad; // :)

    printf("%#lX\n", *i);
    pool_put(&p, i);

    pool_destroy(&p);
    return 0;
}
