#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdbool.h>

#define PATH_LENGTH 4096
#define SUFFIX_LENGTH 256

typedef struct {
	int is_dir;
	char path[PATH_LENGTH];
	char suffix[SUFFIX_LENGTH];
	char parent[PATH_LENGTH];
	char *ext;
	char *name;
} Path;

typedef struct {
	int capacity;
	int count;
	Path *paths;
} PathList;


int is_dir(const char *path);
int safe_mkdir(char *path);
int clear_dir(char *path);
void copy_file(const char *src_path, const char *out_path);
void free_path_list(PathList *path_list);
PathList *create_path_list(const char *dir_path, bool dirs_only);
int test_fs_utils(int argc, char *argv[]);
#endif
