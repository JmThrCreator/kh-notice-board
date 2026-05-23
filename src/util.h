#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#ifdef KH_DEV
#define dev_assert(cond, msg) \
do { \
    if (!(cond)) { \
        fprintf(stderr, "[DEV ASSERT] %s (assertion failed: %s, file %s, line %d)\n", \
                msg, #cond, __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)
#else
#define dev_assert(cond, msg) ((void)0)
#endif

typedef enum {
	OK = 0,
	ERR = 1,
} err_t;

#define try(x) do { if ((x) != OK) return ERR; } while (0)

// arena

typedef struct Arena {
	uint8_t *base;
	size_t capacity;
	size_t offset;
} arena_t;

void arena_init(arena_t *a, size_t capacity) {
	a->base = (uint8_t *)calloc(1, capacity);
	a->capacity = capacity;
	a->offset = 0;
}

void* arena_alloc(arena_t *a, size_t size) {
	if (a->offset + size > a->capacity) return NULL;
	uint8_t *result = a->base + a->offset;
	a->offset += size;
	return result;
}

#define arena_push_array(arena, count, type) \
    (type *)arena_alloc(arena, (count) * sizeof(type))

#define arena_push_struct(arena, type) \
    (type *)arena_alloc(arena, sizeof(type))

void arena_free(arena_t *arena) {
	if (!arena) return;
	free(arena->base);
	arena->base = NULL;
	arena->capacity = 0;
	arena->offset = 0;
}

// app

const char *find_string(char *str, const char *sub_str, unsigned int instance) {
	if (!str || !sub_str) return NULL;
	const size_t length = strlen(str) - 1;
	if (length == 0) return NULL;

	size_t sub_len = strlen(sub_str);
	if (sub_len == 0 || length < sub_len) return NULL;

	char *str_pos = str;
	unsigned int count = 0;

	while (*str_pos) {
		if (*str_pos == *sub_str && strncmp(str_pos, sub_str, sub_len) == 0) {
			count ++;
			if (count == instance) return str_pos;
		}
		str_pos++;
	}

	return NULL;
}

const char *find_string_reverse(const char *str, const char *sub_str, unsigned int instance) {
	if (!str || !sub_str) return NULL;
	const size_t length = strlen(str);
	if (length == 0) return NULL;
	size_t sub_len = strlen(sub_str);
	if (sub_len == 0 || length < sub_len) return NULL;

	const char *str_pos = str + length - sub_len;
	unsigned int count = 0;
	while (str_pos >= str) {
		if (*str_pos == *sub_str && strncmp(str_pos, sub_str, sub_len) == 0) {
			if (++count == instance) return str_pos;
		}
		str_pos--;
	}
	return NULL;
}
