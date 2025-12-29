#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

union __mempool_node {
    union __mempool_node *next;
};

typedef struct {
    arena_t *arena;
    union __mempool_node *list;
    size_t size;
} mempool_t;

void mempool_init(mempool_t *self, arena_t *arena, size_t chunk_size);

void *mempool_get(mempool_t *self);

void mempool_put(mempool_t *self, void *v);

#ifdef __cplusplus
}
#endif

#endif // _MEMPOOL_H_
