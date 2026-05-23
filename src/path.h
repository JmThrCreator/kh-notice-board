#pragma once

#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

#define MAX_PATH_LEN 1024
#define MAX_PATH_LIST_LEN 64
#define MAX_NAME_LEN 256

#ifdef _WIN32
    #define PATH_SEPARATOR '\\'
#else
    #define PATH_SEPARATOR '/'
#endif

// utils

// offset = how many chars after the matched pointer you want to return
// with char: "." and offset 1, ".txt" -> "txt"
const char *find_first_char(const char *str, char c, size_t length, size_t offset) {
	if (!str || length == 0 || offset >= length) return NULL;

	for (size_t i = 0; i < length; i++) {
		if (str[i] == c) {
			size_t found_pos = i + offset;
			if (found_pos >= length) return NULL;
			return &str[found_pos];
		}
	}
	return NULL;
}

// offset = how many chars after the matched pointer you want to return
// with char: "." and offset 1, ".txt" -> "txt"
const char *find_last_char(const char *str, char c, size_t length, size_t offset) {
	if (!str || length == 0 || offset >= length) return NULL;

	for (size_t i = length; i > 0; i--) {

		if (str[i - 1] == c) {
			size_t found_pos = i - 1 + offset;
			if (found_pos >= length) return NULL;
			return &str[found_pos];
		}
	}
	return NULL;
}

bool is_dir_reserved(const char* d_name) {
	if (!d_name) return false;
	if (d_name[0] != '.') return false;
	if (d_name[1] == '\0') return true; // "."
	if (d_name[1] == '.' && d_name[2] == '\0') return true; // ".."
	return false;
}

typedef struct Path {
	size_t len;
	const char *name;
	const char *ext;
	const char *full_path;
	const char *relative_prefix;
} path_t;

typedef struct PathList {
	size_t count;
	size_t capacity;
	path_t *path;
	path_t **items;
} pathlist_t;

typedef enum {
	PATH_NONE  = 0,
	PATH_FOLDER = 1 << 0,
} path_flags_t;

#define path_init(path_ptr, arena, relative_prefix, ...) \
	path_init_n(path_ptr, arena, relative_prefix, PATH_NONE, (sizeof((const char*[]){ __VA_ARGS__ })/sizeof(const char*)), (const char*[]){ __VA_ARGS__ })

#define path_init_flags(path_ptr, arena, relative_prefix, flags, ...) \
	path_init_n(path_ptr, arena, relative_prefix, flags, (sizeof((const char*[]){ __VA_ARGS__ })/sizeof(const char*)), (const char*[]){ __VA_ARGS__ })
err_t path_init_n(path_t *path, arena_t *arena, const char *relative_prefix, path_flags_t flags, size_t n, const char **path_char_list) {
	bool is_folder = (flags & PATH_FOLDER) != 0;
	// construct the full path
	char full_path_buf[MAX_PATH_LEN];
	size_t full_path_len = 0;

	for (size_t i = 0; i < n; ++i) {
		const char *path_char = path_char_list[i];
		dev_assert(path_char, "path invalid");

		size_t path_char_len = strnlen(path_char, MAX_PATH_LEN);
		if (path_char_len == 0) continue;

		// remove trailing path separator
		char last_char = path_char[path_char_len - 1];
		if (last_char == '/' || last_char == '\\') path_char_len--;
		if (path_char_len == 0) continue; // if just a '/'
		
		if (full_path_len + path_char_len > MAX_PATH_LEN) return ERR;
		memcpy(full_path_buf + full_path_len, path_char, path_char_len);

		full_path_len += path_char_len;
		if (i < n - 1) full_path_buf[full_path_len++] = PATH_SEPARATOR;
	}
	full_path_buf[full_path_len] = '\0';

	// alloc
	char *full_path = arena_push_array(arena, full_path_len + 1, char);
	if (!full_path) return ERR;
	memcpy(full_path, full_path_buf, full_path_len + 1);
	full_path[full_path_len] = '\0';

	path->full_path = full_path;
	path->len = full_path_len;

	const char *name_char = find_last_char(path->full_path, '/', path->len, 1);
	if (!name_char) return ERR;
	path->name = name_char;

	size_t name_len = strnlen(path->name, MAX_PATH_LEN);
	if (name_len == 0) return ERR;
	
	path->ext = NULL;
	if (!is_folder) {
		const char *ext_char = find_last_char(path->name, '.', name_len, 1);
		if (ext_char) path->ext = ext_char;
	}

	path->relative_prefix = NULL;
	if (relative_prefix) {
		size_t prefix_len = strnlen(relative_prefix, MAX_PATH_LEN);
		char *relative_prefix_buf = arena_push_array(arena, prefix_len + 1, char);
		if (!relative_prefix_buf) return ERR;
		memcpy(relative_prefix_buf, relative_prefix, prefix_len);
		relative_prefix_buf[prefix_len] = '\0';
		path->relative_prefix = relative_prefix_buf;
	}

	return OK;
}

// pathlist utils

bool path_ext_matches(const path_t *path, const char **exts, size_t ext_count) {
	if (!path) return false;
	if (!path->ext) return true;

	for (size_t i = 0; i < ext_count; i++) {
		const char *ext = exts[i];
		if (!ext) continue;

		// allow ".pdf" or "pdf"
		const char *path_ext = (*path->ext == '.') ? path->ext + 1 : path->ext;
		const char *want_ext = (*ext == '.') ? ext + 1 : ext;

		if (strcmp(path_ext, want_ext) == 0) {
		    return true;
		}
	}
	return false;
}

static const char *NO_EXT[] = { "" };

void pathlist_filter_by_ext(pathlist_t *list, const char **exts, size_t ext_count) {
	if (!list || !list->items) return;

	size_t write = 0;
	for (size_t read = 0; read < list->count; read++) {
		path_t *path = list->items[read];
        	if (path_ext_matches(path, exts, ext_count)) {
			list->items[write++] = path;
		}
	}

	list->count = write;
}

