#include "platform/platform.h"
#include "platform/posix.h"

#include "util.h"
#include "path.h"
#include "app.h"
#include "mongoose.c"

#define PROJECT_NAME "kh_notice_board"

void open_browser(const char *url) {
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "open \"%s\"", url);
	system(cmd);
}

err_t setup_config(path_t *config_path, arena_t *arena) {
	const char *home = getenv("HOME");
	if (home == NULL) return ERR;
	try(path_init(config_path, arena, "", home, "Library", "Application Support", PROJECT_NAME));
	return create_dir(config_path);
}

err_t setup_cache(path_t *cache_path, arena_t *arena) {
	const char *home = getenv("HOME");
	if (home == NULL) return ERR;
	try(path_init(cache_path, arena, "", home, "Library", "Caches", PROJECT_NAME));
	return create_dir(cache_path);
}

int main(int argc, char **argv) {
	app_main();
}
