#ifndef CACHE_H
#define CACHE_H

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "uthash.h"

#include "fs_utils.h"
#include "pdf_utils.h"

#define MAX_CONCURRENT_PROCESSES 30

typedef struct {
	  char path_name[PATH_LENGTH];
	  UT_hash_handle hh;
} PathNameHashEntry;

int setup_cache(char cache_path[]);
int update_cache_folders(char *cache_path, PathList *input_dir_list);
void update_single_cache_folder(char *cache_path, char *input_dir_path, int *num_processes, pid_t *pids);
int update_cache(char *input_path);

//int main(int argc, char *argv[]);

#endif
