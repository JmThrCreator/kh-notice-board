#pragma once

#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "path.h"

// process

#define MAX_THREADS 16
#define MAX_CONFIG_LINE 1024

typedef struct Thread kh_thread_t;
typedef void *(*thread_func_t)(void *);

err_t thread_create(arena_t *arena, kh_thread_t **out_thread, thread_func_t func, void *arg);
err_t thread_join(kh_thread_t *thread);
err_t thread_detach(kh_thread_t *thread);

// file system

typedef enum {
	PATHLIST_DIRS_ONLY  = 1u << 0,
	PATHLIST_RECURSIVE  = 1u << 1,
	PATHLIST_FILES_ONLY = 1u << 2,
} pathlist_flags_t;
err_t pathlist_init(pathlist_t *path_list, arena_t *arena, const char *full_path, unsigned flags);

bool path_exists(path_t *path);
err_t create_dir(path_t *path);
err_t clear_dir(path_t *dir_path, arena_t *scratch);
err_t remove_empty_dir(path_t *dir_path);
err_t copy_file(path_t *src_path, path_t *dst_path);
err_t is_dir(const char *path);

// cache

err_t setup_cache(path_t *cache_path, arena_t *arena);
err_t setup_config(path_t *cache_path, arena_t *arena);

// config

#define DEFAULT_INPUT_DIR "{\"input_dir\": \"\"}"

err_t write_config(path_t *file_path, const char *file_text);
err_t read_config(path_t *file_path, arena_t *arena, char **out_text);

// browser

void open_browser(const char *url);
