#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h> // fork, wait
#include <sys/types.h> // pid_t
#include <sys/wait.h> // waitpid

#include "uthash.h"

#include "fs_utils.h"
#include "pdf_utils.h"

#define MAX_CONCURRENT_PROCESSES 15

typedef struct {
	  char path_name[PATH_LENGTH];
	  UT_hash_handle hh;
} PathNameHashEntry;

int setup_cache(char cache_path[]) {
	#ifdef _WIN32
		const char *appdata = getenv("APPDATA");
		if (appdata == NULL) {
			fprintf(stderr, "Failed to get APPDATA environment variable.\n");
			return 1;
		}
		snprintf(cache_path, PATH_LENGTH, "%s\\kh-notice-board", appdata);
	#elif defined(__APPLE__)
		const char *home = getenv("HOME");
		if (home == NULL) {
			fprintf(stderr, "Failed to get HOME environment variable.\n");
			return 1;
		}
		snprintf(cache_path, PATH_LENGTH, "%s/Library/Caches/kh-notice-board", home);
	#else
		const char *xdg_cache = getenv("XDG_CACHE_HOME");
		if (xdg_cache != NULL) {
			snprintf(cache_path, PATH_LENGTH, "%s/kh-notice-board", xdg_cache);
		} else {
			const char *home = getenv("HOME");
			if (home == NULL) {
				fprintf(stderr, "Failed to get HOME environment variable.\n");
				return 1;
			}
			snprintf(cache_path, PATH_LENGTH, "%s/.cache/kh-notice-board", home);
		}
	#endif
	return safe_mkdir(cache_path);
}

int update_cache_folders(char *cache_path, PathList *input_dir_list) {
	PathList *cache_dir_list = create_path_list(cache_path, false);

	// create hash table for input dirs
	PathNameHashEntry *input_hash_entries = malloc(input_dir_list->count * sizeof(PathNameHashEntry));
	if (!input_hash_entries) {
		fprintf(stderr, "Failed to allocate memory for hash table entries.\n");
		free_path_list(cache_dir_list);
		return 1;
	}
	PathNameHashEntry *input_hash = NULL;
	for (int i = 0; i < input_dir_list->count; i++) {
		Path *input_dir = &input_dir_list->paths[i];
		strcpy(input_hash_entries[i].path_name, input_dir->name);
		HASH_ADD_STR(input_hash, path_name, &input_hash_entries[i]);
	}

	// add new folders to cache
	for (int i = 0; i < input_dir_list->count; i++) {
		Path *input_dir = &input_dir_list->paths[i];
		char new_cache_dir[PATH_LENGTH];
		snprintf(new_cache_dir, sizeof(new_cache_dir), "%s/%s", cache_path, input_dir->name);
		safe_mkdir(new_cache_dir);
	}

	// remove folders not in input	
	for (int i = 0; i < cache_dir_list->count; i++) {
		Path *cache_dir = &cache_dir_list->paths[i];
		PathNameHashEntry *entry;
		HASH_FIND_STR(input_hash, cache_dir->name, entry);
		if (entry == NULL) {
			rmdir(cache_dir->path);
		}
	}

	// cleanup
	free(input_hash_entries);
	free_path_list(cache_dir_list);

	return 0;
}

void update_single_cache_folder(char *cache_path, char *input_dir_path, int *num_processes, pid_t *pids) {
	PathList *input_file_path_list = create_path_list(input_dir_path, false);

	for (int j = 0; j < input_file_path_list->count; j++) {
		Path *input_file_path = &input_file_path_list->paths[j];
		if (input_file_path->ext == NULL || strstr("jpg,jpeg,png,pdf", input_file_path->ext) == NULL) continue; // filter
	
		char cache_file_dir_path[PATH_LENGTH];
		snprintf(
			cache_file_dir_path, sizeof(cache_file_dir_path), "%s/%s/%s",
			cache_path, basename(input_dir_path), input_file_path->suffix);
		safe_mkdir(cache_file_dir_path);

		pid_t pid = fork();
		if (pid == -1) {
			perror("Fork failed");
			continue;
		}
		if (pid == 0) {
			if (strcmp(input_file_path->ext, "pdf") == 0) {
				pdf_to_png(input_file_path, cache_file_dir_path);
			} else if (strstr("jpg,jpeg,png", input_file_path->ext)) {
				char cache_file_path[PATH_LENGTH];
				snprintf(
					cache_file_path, sizeof(cache_file_path), "%s/%s/%s/1.%s", 
					cache_path, basename(input_dir_path), input_file_path->suffix, input_file_path->ext);
				copy_file(input_file_path->path, cache_file_path);
			}
			//printf("%i FREED (%s)\n", getpid(), input_pdf_path->name);
			exit(0);
		} else {
			if (*num_processes < MAX_CONCURRENT_PROCESSES) {
				pids[*num_processes] = pid;
				(*num_processes)++;
			};
			
			while (*num_processes >= MAX_CONCURRENT_PROCESSES) {
				pid_t waited_pid = waitpid(-1, NULL, WNOHANG);  // wait for any child to finish
				if (waited_pid < 0) {
					perror("waitpid");
					break;
				}
				for (int idx = 0; idx < *num_processes; idx++) {
					if (pids[idx] != waited_pid) {
						continue;
					}
					// shift PIDs after waited_pid down list
					for (int shift_idx = idx; shift_idx < *num_processes - 1; shift_idx++) {
						pids[shift_idx] = pids[shift_idx + 1];
					}
					(*num_processes)--;
					break;
				}
				usleep(10000); // 10ms
			}
		}
	}
	free_path_list(input_file_path_list);
}

int update_cache(char *input_path) {
	// set up cache dir
	char cache_path[PATH_LENGTH];
	int result = setup_cache(cache_path);
	if (result != 0 || strlen(cache_path) == 0) {
		fprintf(stderr, "Failed to access or create cache directory.\n");
		return 1;
	}
	
	// create input dir list
	PathList *input_dir_list = create_path_list(input_path, true);
	if (!input_dir_list || input_dir_list->paths == NULL) {
		fprintf(stderr, "Input directory empty.\n");
		return 1;
	}

	// update cache folders
	result = update_cache_folders(cache_path, input_dir_list);
	if (result != 0) {
		fprintf(stderr, "Failed to update cache folders.\n");
		return 1;
	}

	// clear top level dirs (kh folders)
	PathList *cache_dir_list = create_path_list(cache_path, true);
	for (int i = 0; i < cache_dir_list->count; i++) {
		Path *cache_dir = &cache_dir_list->paths[i];
		clear_dir(cache_dir->path);
	}

	printf("Converting PDFs...\n");
	int num_processes = 0;
	pid_t pids[MAX_CONCURRENT_PROCESSES] = {0};
	for (int i = 0; i < input_dir_list->count; i++) {
		Path *input_dir = &input_dir_list->paths[i];
		update_single_cache_folder(cache_path, input_dir->path, &num_processes, pids);
	}
	for (int i = 0; i < num_processes; i++) {
		if (waitpid(pids[i], NULL, 0) == -1) {
			perror("waitpid failed");
		}
	}
	printf("Finished\n");
	free_path_list(input_dir_list);
	free_path_list(cache_dir_list);
	return 0;
}

/*
int main() {
	// resolve absolute path
	char input_path[PATH_LENGTH];
	if (realpath(argv[1], input_path) == NULL) {
		perror("realpath");
		return 1;
	}
	
	// set up cache dir
	char cache_path[PATH_LENGTH];
	int result = setup_cache(cache_path);
	if (result != 0 || strlen(cache_path) == 0) {
		fprintf(stderr, "Failed to access or create cache directory.\n");
		return 1;
	}

	// create input dir list
	PathList *input_dir_list = create_path_list(input_path, true);
	if (!input_dir_list || input_dir_list->paths == NULL) {
		fprintf(stderr, "Input directory empty.\n");
		return 1;
	}

	// update cache folders
	result = update_cache_folders(cache_path, input_dir_list);
	if (result != 0) {
		fprintf(stderr, "Failed to update cache folders.\n");
		return 1;
	}

	result = update_cache(cache_path, input_dir_list);
	if (result != 0) {
		fprintf(stderr, "Failed to update cache.\n");
		return 1;
	}

	free_path_list(input_dir_list);

	return 0;
}
*/