#include <stdio.h> // printf, perror
#include <dirent.h> // DIR, dirent, opendir, readdir, closedir
#include <sys/stat.h> // stat, S_ISDIR
#include <unistd.h> // rmdir
#include <string.h>
#include <stdlib.h> // free
#include <libgen.h> // basename
#include <stdbool.h>

#define INITIAL_PATH_LIST_CAPACITY 100
#define PATH_LIST_CAPACITY_INCREMENT 100
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

// misc

int is_dir(const char *path) {
	struct stat statbuf;
	return stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode);
}

int safe_mkdir(char *path) {
	if (!is_dir(path) && mkdir(path, 0755) == -1) {
		perror("mkdir");
		return 1;
	}
	return 0;
}

int clear_dir(char *dir_path) {
	struct dirent *entry;
	DIR *dp = opendir(dir_path);
	if (!dp) return 1;

	while ((entry = readdir(dp)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

		char path[PATH_LENGTH];
		snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
		if (is_dir(path) != 0) {
			clear_dir(path);
			if (rmdir(path) != 0) {
				perror("rmdir");
			}
		} else {
			unlink(path);
		}
	}
	closedir(dp);
	return 0;
}


char *get_path_ext(const char *path) {
	char *ext = strrchr(path, '.');
	if (ext == NULL || *(ext + 1) == '\0') return NULL;
	return ext + 1;
}

char *get_path_suffix(char *path) {
	char *suffix = basename(path);
	const char *ext = strchr(suffix, '.');
	if (!ext) return strdup(suffix);

	if (ext - suffix == 0) return strdup("");
	return strndup(suffix, ext - suffix);
}

void copy_file(const char *src_path, const char *out_path) {
	FILE *src_file = fopen(src_path, "rb");
	if (src_file == NULL) {
		perror("Error opening source file");
		return;
	}

	FILE *out_file = fopen(out_path, "wb");
	if (out_file == NULL) {
		perror("Error opening output file");
		fclose(src_file);
		return;
	}

	char buffer[1024];
	size_t bytes_read;
	while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
		fwrite(buffer, 1, bytes_read, out_file);
	}

	fclose(src_file);
	fclose(out_file);
}

bool has_multi_files(const char *path) {
	DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return false;
    }

	struct dirent *entry;
    int count = 0;
	char full_path[PATH_LENGTH];

    while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		snprintf(full_path, PATH_LENGTH, "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
            count++;
			if (count > 1) {
				closedir(dir);
				return true;
			}
        }
    }
	closedir(dir);
	return false;
}

// path list

void free_path_list(PathList *path_list) {
	if (!path_list) return;
	free(path_list->paths);
	free(path_list);
}

PathList *create_path_list(const char *dir_path, bool dirs_only) {
	// allocate memory
	PathList *path_list = malloc(sizeof(PathList));
	if (!path_list) {
		perror("Failed to allocate memory for PathList");
		return NULL;
	}

	path_list->capacity = INITIAL_PATH_LIST_CAPACITY;
	path_list->count = 0;
	path_list->paths = malloc(path_list->capacity * sizeof(Path));
	if (!path_list->paths) {
		perror("Failed to allocate memory for paths");
		free(path_list);
		return NULL;
	}

	DIR *dir = opendir(dir_path);
	if (!dir) {
		perror("Failed to open directory");
		free_path_list(path_list);
		return NULL;
	}
	
	struct dirent *entry;
	char full_path[PATH_LENGTH];
	
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		snprintf(full_path, PATH_LENGTH, "%s/%s", dir_path, entry->d_name);
		if (dirs_only && !is_dir(full_path))
			continue;

		// resize
		if (path_list->count >= path_list->capacity) {
			int new_capacity = path_list->capacity + PATH_LIST_CAPACITY_INCREMENT;
			Path *new_paths = realloc(path_list->paths, new_capacity * sizeof(Path));
			if (!new_paths) {
				perror("Failed to resize paths array");
				closedir(dir);
				free_path_list(path_list);
				return NULL;
			}
			path_list->paths = new_paths;
			path_list->capacity = new_capacity;
		}
		// init new path
		Path *new_path = &path_list->paths[path_list->count++];

		new_path->is_dir = is_dir(full_path);
		strncpy(new_path->path, full_path, PATH_LENGTH);
		new_path->ext = get_path_ext(entry->d_name);

		char *base = basename(new_path->path);
		new_path->name = strdup(base);

		snprintf(new_path->suffix, SUFFIX_LENGTH, "%s", get_path_suffix(entry->d_name));
		strncpy(new_path->parent, dir_path, PATH_LENGTH);
	}

	closedir(dir);
	return path_list;
}

int test_fs_utils(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage %s <directory_path>\n", argv[0]);
		return 1;
	}

	char input_path[PATH_LENGTH];
	if (realpath(argv[1], input_path) == NULL) {
		perror("realpath");
		return 1;
	}


	PathList *input_path_list = create_path_list(input_path, false);
	if (!input_path_list) {
		return 1;
	}
	printf("Path List:\n\n");
	for (int i = 0; i < input_path_list->count; i++) {
		printf("Path %d: %s, Is Directory: %d, Extension: %s, Name: %s, Suffix: %s, Parent: %s\n",
			i + 1, input_path_list->paths[i].path, input_path_list->paths[i].is_dir, input_path_list->paths[i].ext, input_path_list->paths[i].name,
			input_path_list->paths[i].suffix, input_path_list->paths[i].parent);
	}
	return 0;
}
