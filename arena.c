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

#ifdef __cplusplus
extern "C" {
#endif

#include "arena.h"

#ifndef NULL
    #define NULL ((void*)0)
#endif

#ifndef offsetof
    #define offsetof(_type, _field) ((arena_size_t)(&(((_type*)NULL)->_field)))
#endif

#define ALIGN_UP(p, alignment) \
    ((void*)((((arena_uintptr_t)(p)) + ((alignment) - 1)) & (~((alignment) - 1))))


typedef unsigned char byte;

typedef struct arena_buffer {
    struct arena_buffer *next;
    arena_size_t ptr;
    /* a work-around for zero-sized arrays (C99) in C89 */
    byte buf[1];
} arena_buffer_t;

/* A simple macro to cast a pointer to arena_buffer_t.
 * Use case: less typing :) */
#define B(p) ((arena_buffer_t*)(p))

typedef struct allochdr {
    arena_size_t size;
    arena_size_t padding;
} allochdr_t;
#define ahs (sizeof(allochdr_t))

static arena_buffer_t *buffer_new(arena_size_t cap, arena_allocator_fn alloc_fn) {
    arena_buffer_t *b;
    /* use offsetof instead of sizeof because sizeof will include the padding
     * at the end of arena_buffer_t and size of the buf field itself
     * */
    b = alloc_fn(NULL, offsetof(arena_buffer_t, buf) + cap);
    if (!b)
        return NULL;
    b->ptr = 0;
    b->next = NULL;
    return b;
}

int arena_init_(arena_t *arena,
                arena_size_t cap,
                arena_size_t alignment,
                arena_allocator_fn alloc_fn,
                arena_size_t flags)
{
    arena->first = buffer_new(cap, alloc_fn);
    if (!arena->first)
        return 0;
    arena->current = arena->first;

    arena->flags = flags;
    arena->alignment = alignment;
    arena->allocator = alloc_fn;
    arena->cap = cap;

    return 1;
}

arena_size_t arena_last_size(arena_t *arena) {
    allochdr_t hdr;
    arena_buffer_t *current;
    if (!(arena->flags & ARENA_STACK) || B(arena->current)->ptr == 0)
        return 0;
    current = arena->current;
    hdr = *(allochdr_t*) &(current->buf[current->ptr - ahs]);
    return hdr.size;
}

void *arena_pop(arena_t *arena) {
    allochdr_t hdr;
    arena_buffer_t *current;

    current = arena->current;

    if (!(arena->flags & ARENA_STACK) || current->ptr == 0)
        return NULL;

    /* set ptr to header location */
    current->ptr -= ahs;

    /* read the allocation header */
    hdr = *(allochdr_t*) &(current->buf[current->ptr]);
    /* reset the current buffer's pointer */
    current->ptr -= hdr.size + hdr.padding;
    /* return the aligned address */
    return (&current->buf[current->ptr]) + hdr.padding;
}

void *arena_alloc_align(arena_t *arena, arena_size_t size, arena_size_t alignment) {
    allochdr_t *hdr;
    byte *raw, *aligned;
    arena_buffer_t *current, *new_buffer;
    arena_size_t padding, required_space;

    if (size == 0)
        return NULL;

    current = arena->current;
    raw = &current->buf[current->ptr];
    aligned = ALIGN_UP(raw, alignment);
    padding = aligned - raw;
    required_space = size + padding;

    /* if ARENA_STACK is set, include allocation header (metadata) in
     * required_space
     * */
    if (arena->flags & ARENA_STACK)
        required_space += ahs;

    if (required_space > (arena->cap - current->ptr)) {
        if (arena->flags & ARENA_FIXED)
            return NULL;

        new_buffer = buffer_new(arena->cap, arena->allocator);
        if (!new_buffer)
            return NULL;

        B(arena->current)->next = new_buffer;
        arena->current = new_buffer;

        /* reinitialize allocation info */
        current = arena->current;
        raw = current->buf;
        aligned = ALIGN_UP(raw, alignment);
        padding = aligned - raw;
    }

    if (arena->flags & ARENA_STACK) {
        /* Store allocation metadata AFTER the allocated block */
        hdr = (allochdr_t*)(aligned + size);
        hdr->size = size;
        hdr->padding = padding;
        current->ptr += ahs;
    }

    current->ptr += size + padding;
    return aligned;
}

int arena_is_empty(arena_t *arena) {
    arena_buffer_t *f;
    f = B(arena->first);
    return ((f->next == NULL) && (f->ptr == 0));
}

arena_size_t arena_pos(arena_t *arena) {
    return (B(arena->current)->ptr);
}

static void arena_buffers_free(arena_buffer_t *first, arena_allocator_fn alloc_fn) {
    arena_buffer_t *tmp, *next;
    tmp = first;
    while (tmp) {
        next = tmp->next;
        alloc_fn(tmp, 0);
        tmp = next;
    }
}

void arena_reset(arena_t *arena, int how) {
    if (how == ARENA_RESET_ALL) {
        arena_buffers_free(B(arena->first)->next, arena->allocator);
        B(arena->first)->ptr = 0;
        B(arena->first)->next = NULL;
        arena->current = arena->first;
    } else {
        B(arena->current)->ptr = 0;
    }
}

void arena_deinit(arena_t *arena) {
    arena_buffers_free(arena->first, arena->allocator);
}

#ifdef __cplusplus
}
#endif
