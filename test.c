#include <stdio.h>
#include <assert.h>
#include "arena.h"

int main(void) {
    arena_t *a;
    long *p, *z;

    printf("Arena version %d.%d.%d\n",
           ARENA_VERSION_MAJOR,
           ARENA_VERSION_MINOR,
           ARENA_VERSION_PATCH);

    arena_config_t arena_config = ARENA_DEFAULT_CONFIG;
    if (! (a = arena_new(&arena_config))) {
        perror("arena");
        return 1;
    }

    p = (long*) arena_alloc(a, sizeof(*p));
    *p = 111;

    arena_pop(a, sizeof(*p));

    z = (long*) arena_alloc(a, sizeof(*z));

    assert(p == z);
    assert(*p == 111);

    arena_destroy(a);
    puts("OK!");
    return 0;
}
