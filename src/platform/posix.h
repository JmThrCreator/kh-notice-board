#pragma once

#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

#include "util.h"
#include "platform.h"

extern char **environ;

// threads

struct Thread {
	pthread_t handle;
};

err_t thread_create(arena_t *arena, kh_thread_t **out_thread, thread_func_t func, void *arg) {
	kh_thread_t *thread = arena_push_struct(arena, kh_thread_t);
	if (!thread) return ERR;

	if (pthread_create(&thread->handle, NULL, func, arg) != 0) return ERR;

	*out_thread = thread;
	return OK;
}

err_t thread_join(kh_thread_t *thread) {
    if (!thread) return ERR;
    pthread_join(thread->handle, NULL);
    return OK;
}

err_t thread_detach(kh_thread_t *thread) {
	if (!thread) return ERR;
	if (pthread_detach(thread->handle) != 0) return ERR;
	return OK;
}

// file system

#define MAX_PATHS 100
#define PATH_SEPARATOR '/'

bool path_exists(path_t *path) {
	struct stat statbuf;
	return (stat(path->full_path, &statbuf) == 0);
}

err_t is_dir(const char *path) {
	struct stat statbuf;
	try(stat(path, &statbuf) != 0);
	return S_ISDIR(statbuf.st_mode);
}

err_t pathlist_collect(pathlist_t *path_list, arena_t *arena, const char *full_path, const char *relative_prefix, unsigned flags) {
	bool dirs_only = (flags & PATHLIST_DIRS_ONLY) != 0;
	bool recursive = (flags & PATHLIST_RECURSIVE) != 0;
	bool files_only = (flags & PATHLIST_FILES_ONLY) != 0;

	dev_assert(!(dirs_only && files_only), "DIRS_ONLY and FILES_ONLY both set");

	DIR *dir = opendir(full_path);
	if (!dir) return ERR;
	struct dirent *entry;

	for (; path_list->count < MAX_PATHS;) {
		if ((entry = readdir(dir)) == NULL) break;
		if (is_dir_reserved(entry->d_name)) continue;

		char child_full_path[MAX_PATH_LEN];
		int n = snprintf(child_full_path, MAX_PATH_LEN, "%s%c%s", full_path, PATH_SEPARATOR, entry->d_name);
		if (n < 0 || n >= MAX_PATH_LEN) {
			closedir(dir);
			return ERR;
		}

		bool add_entry = true;
		if (dirs_only && !is_dir(child_full_path)) add_entry = false;
		if (files_only && is_dir(child_full_path)) add_entry = false;

		char child_relative_prefix[MAX_PATH_LEN];
		if (relative_prefix[0] == '\0') {
			snprintf(child_relative_prefix, MAX_PATH_LEN, "%s", entry->d_name);
		} else {
			snprintf(child_relative_prefix, MAX_PATH_LEN, "%s%c%s", relative_prefix, PATH_SEPARATOR, entry->d_name);
		}
		
		if (add_entry) {
			path_t *child_path = arena_push_struct(arena, path_t);
			path_flags_t flag = is_dir(child_full_path) ? PATH_FOLDER : PATH_NONE;
			if (path_init_flags(child_path, arena, relative_prefix, flag, child_full_path) == ERR) {
				closedir(dir);
				return ERR;
			}

			path_list->items[path_list->count++] = child_path;
		}
		
		if (recursive && is_dir(child_full_path)) {
			if (pathlist_collect(path_list, arena, child_full_path, child_relative_prefix, flags) == ERR) {
				return ERR;
			}
		}
	}
	closedir(dir);

	return OK;
}

err_t pathlist_init(pathlist_t *path_list, arena_t *arena, const char *full_path, unsigned flags) {
	path_list->count = 0;
	path_list->capacity = MAX_PATHS;

	// root path
	path_list->path = arena_push_struct(arena, path_t);
	try(path_init_flags(path_list->path, arena, "", PATH_FOLDER, full_path));

	// storage
	path_list->items = arena_push_array(arena, MAX_PATHS, path_t*);
	if (!path_list->items) return ERR;

	return pathlist_collect(path_list, arena, full_path, "", flags);
}

err_t create_dir(path_t *path) {
	if (path_exists(path)) return OK;
	//dev_assert(path->is_dir, "create_dir called on a file path");
	return mkdir(path->full_path, 0755);
}

err_t clear_dir(path_t *dir_path, arena_t *scratch) {
	DIR *dp = opendir(dir_path->full_path);
	if (!dp) return ERR;

	struct dirent *entry;
	while ((entry = readdir(dp)) != NULL) {
		if (is_dir_reserved(entry->d_name)) continue;
		
		path_t path;
		if (path_init(&path, scratch, "", dir_path->full_path, entry->d_name) == ERR) {
        		closedir(dp);
        		return ERR;
        	}

		if (is_dir(path.full_path)) {
			if (clear_dir(&path, scratch) == ERR) {
                		closedir(dp);
                		return ERR;
			}
			if (rmdir(path.full_path) != 0) {
				closedir(dp);
				return ERR;
			}
		} else {
			if (unlink(path.full_path) != 0) {
				closedir(dp);
				return ERR;
			}
		}
	}
	closedir(dp);
	return OK;
}

err_t remove_empty_dir(path_t *dir_path) {
	return rmdir(dir_path->full_path);
}

// files

err_t write_config(path_t *file_path, const char *file_text) {
	FILE *file = fopen(file_path->full_path, "w");
	if (!file) return ERR;

	size_t len = strnlen(file_text, MAX_CONFIG_LINE);
	if (len >= MAX_CONFIG_LINE) {
		fclose(file);
		return ERR;
	}
	size_t written = fwrite(file_text, 1, len, file);
	if (fclose(file) != 0) return ERR;
	if (written != len) return ERR;
	return OK;
}

err_t read_config(path_t *file_path, arena_t *arena, char **out_text) {
	FILE *file = fopen(file_path->full_path, "rb");
	if (!file) return ERR;

	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		return ERR;
	}

	long size = ftell(file);
	if (size < 0) {
		fclose(file);
		return ERR;
	}

	rewind(file);

	char *buffer = arena_alloc(arena, size + 1);
	if (fread(buffer, 1, size, file) != (size_t)size) {
		fclose(file);
		return ERR;
	}

	buffer[size] = '\0';
	fclose(file);

	*out_text = buffer;
	return OK;
}
