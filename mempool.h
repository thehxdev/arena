#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef union __mempool_chunk __mempool_chunk_t;

typedef struct mempool {
    arena_t *arena;
    __mempool_chunk_t *freelist;
} mempool_t;

void mempool_init(mempool_t *self, arena_t *arena, size_t chunk_size);

void *mempool_get(mempool_t *self);

void mempool_put(mempool_t *self, void *v);

#ifdef __cplusplus
}
#endif

#endif // _MEMPOOL_H_
