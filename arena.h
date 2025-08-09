/*
 * Standalone and zero-dependency arena allocator implementation in C89.
 * Repository: https://github.com/thehxdev/arena
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _ARENA_H_
#define _ARENA_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
    #include <windows.h>

    typedef INT_PTR _arena_intptr_t;
    typedef UINT_PTR _arena_uintptr_t;

    typedef SIZE_T _arena_size_t;
    typedef SSIZE_T _arena_ssize_t;
#else
    typedef long _arena_intptr_t;
    typedef unsigned long _arena_uintptr_t;

    typedef long _arena_ssize_t;
    typedef unsigned long _arena_size_t;
#endif

#define arena_uintptr_t _arena_uintptr_t
#define arena_intptr_t  _arena_intptr_t
#define arena_size_t    _arena_size_t
#define arena_ssize_t   _arena_ssize_t

#if 0
void *example_allocator(void *p, unsigned long size) {
    if (size == 0)
        free(p);
    return malloc(size);
}
#endif
/*
 * The allocator function MUST behave like the example_allocator that described
 * above. There is no re-allocation in this library, but one practical example
 * of such an allocator is glibc's `realloc` function.
 */
typedef void*(*arena_allocator_fn)(void *p, arena_size_t size);

#ifndef ARENA_DEFAULT_ALIGNMENT
    #define ARENA_DEFAULT_ALIGNMENT sizeof(arena_uintptr_t)
#endif

enum {
    ARENA_DEFAULT = 0,
    /* arena will be fixed in size and not grow in case of space limitation */
    ARENA_FIXED   = (1 << 0),
    /* arena will behave like a stack and keeps metadata after each allocation */
    ARENA_STACK   = (1 << 1)
};
typedef struct arena {
    /* all the fields are read-only to the user */
    arena_size_t flags, cap, alignment;
    void *first, *current;
    arena_allocator_fn allocator;
} arena_t;

int arena_init_(arena_t *arena,
                arena_size_t cap,
                arena_size_t alignment,
                arena_allocator_fn alloc_fn,
                arena_size_t flags);

/* helper macro to use default alignment value for arena allocations */
#define arena_init(arena, cap, alloc_fn, flags) \
    arena_init_((arena), (cap), ARENA_DEFAULT_ALIGNMENT, (alloc_fn), (flags))

/* Allocate memory on arena with specified alignment. The alignment value  MUST
 * be a power of 2 */
void *arena_alloc_align(arena_t *arena, arena_size_t size, arena_size_t alignment);

/* Helper macro to use arena's alignment value for allocations */
#define arena_alloc(arena, size) \
    arena_alloc_align((arena), (size), (arena)->alignment)

/* Is arena empty? May become useful for `pool` implementations. */
int arena_is_empty(arena_t *arena);

/* Get size of last item in arena. Only works if ARENA_STACK flag is specified.
 * Otherwise always returns zero.
 * */
arena_size_t arena_last_size(arena_t *arena);

/* Get a pointer to last item and remove that from stack. Pushing new items
 * will overwrite it's data so user MUST copy data befor ANY new push
 * operation. Only works if ARENA_STACK flag is specified. Otherwise returns
 * NULL does nothing.
 * */
void *arena_pop(arena_t *arena);

enum {
    /* if arena has more than one buffer, just reset the last (current) buffer */
    ARENA_RESET_LAST,
    /* if arena has more than one buffer, free those, keep the first buffer and
     * just reset it's pointer
     * */
    ARENA_RESET_ALL
};
void arena_reset(arena_t *arena, int how);

void arena_deinit(arena_t *arena);

#ifdef __cplusplus
}
#endif

#endif /* _ARENA_H_ */
