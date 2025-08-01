#include "arena.h"
#include <stdlib.h>

Arena *arena_create(size_t size) {
    Arena *arena = malloc(sizeof(Arena));
    if (!arena) return NULL;

    arena->memory = malloc(size);
    if (!arena->memory) {
        free(arena);
        return NULL;
    }
    arena->size = size;
    arena->offset = 0;
    return arena;
}

void *arena_alloc(Arena *arena, size_t bytes) {
    if (arena->offset + bytes > arena->size) {
        return NULL;
    }

    void *ptr = arena->memory + arena->offset;
    arena->offset += bytes;

    return ptr;
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
}

void arena_free(Arena *arena) {
    free(arena->memory);
    free(arena);
}