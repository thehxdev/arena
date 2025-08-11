#include <stdio.h>
#include <assert.h>
#include "arena.h"

int main(void) {
    arena_t *a;
    long *p, *z;

    arena_config_t arena_config = ARENA_DEFAULT_CONFIG;

    if (! (a = arena_new(&arena_config))) {
        perror("arena");
        return 1;
    }

    p = arena_alloc(a, sizeof(*p));
    *p = 111;

    arena_pop(a, sizeof(*p));

    z = arena_alloc(a, sizeof(*z));
    printf("value = %ld\n", *p);

    assert(p == z);
    arena_deinit(a);
    return 0;
}
