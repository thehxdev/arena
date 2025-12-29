#include "arena.h"
#include "mempool.h"

#ifdef __cplusplus
extern "C" {
#endif

void mempool_init(mempool_t *self, arena_t *arena, size_t size) {
    size = (size >= sizeof(union __mempool_node)) ? size : sizeof(union __mempool_node);
    *self = (mempool_t){
        .arena = arena,
        .list = NULL,
        .size = size
    };
}

void *mempool_get(mempool_t *self) {
    union __mempool_node *c;
    if (!self->list)
        return arena_alloc(self->arena, self->size);
    c = self->list;
    self->list = c->next;
    return c;
}

void mempool_put(mempool_t *self, void *v) {
    union __mempool_node *c = (union __mempool_node*) v;
    c->next = self->list;
    self->list = c;
}

#ifdef __cplusplus
}
#endif
