#define _GNU_SOURCE

#include "platform/platform.h"
#include "platform/posix.h"

#include "util.h"
#include "path.h"
#include "app.h"
#include "mongoose.c"


#define PROJECT_NAME "kh_notice_board"

void open_browser(const char *url) {
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "xdg-open \"%s\"", url);
	system(cmd);
}

err_t get_xdg_dir(char *buf, size_t len, const char *xdg_env_var, const char *fallback_suffix) {
	const char *home_dir = getenv(xdg_env_var);
	if (home_dir != NULL) {
		snprintf(buf, len, "%s", home_dir);
        	return OK;
	}
	const char *home = getenv("HOME");
	if (home == NULL) return ERR;
	snprintf(buf, len, "%s/%s", home, fallback_suffix);
	return OK;
}

err_t setup_config(path_t *config_path, arena_t *arena) {
	char config_root_str[MAX_PATH_LEN];
	try(get_xdg_dir(config_root_str, sizeof(config_root_str), "XDG_CONFIG_HOME", ".config"));

	path_t config_root;
	try(path_init(&config_root, arena, "", config_root_str));
	if (!path_exists(&config_root)) try(create_dir(&config_root));

	try(path_init(config_path, arena, "", config_root_str, PROJECT_NAME));
	return create_dir(config_path);
}

err_t setup_cache(path_t *cache_path, arena_t *arena) {
	char cache_root_str[MAX_PATH_LEN];
	try(get_xdg_dir(cache_root_str, sizeof(cache_root_str), "XDG_CACHE_HOME", ".cache"));

	path_t cache_root;
	try(path_init(&cache_root, arena, "", cache_root_str));
	if (!path_exists(&cache_root)) try(create_dir(&cache_root));

	try(path_init(cache_path, arena, "", cache_root_str, PROJECT_NAME));
	return create_dir(cache_path);
}

int main(int argc, char **argv) {
	app_main();
}


