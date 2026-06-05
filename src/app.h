#pragma once

#include <stdio.h>

#include "util.h"
#include "cache.h"
#include "config.h"
#include "mongoose.h"
#include "platform/platform.h"

// assets
#include "index_html.h"
#include "folder_html.h"
#include "file_html.h"
#include "error_html.h"
#include "refresh_svg.h"
#include "style_css.h"
#include "script_js.h"
#include "back_svg.h"

#define PLACEHOLDER_STR "{{ placeholder }}"
#define PLACEHOLDER_STR_LEN 17

// embedded assets

typedef struct EmbeddedAsset {
	const char *uri;
	const unsigned char *data;
	size_t size;
	const char *content_type;
} embedded_asset_t;

typedef struct Context {
	path_t *cache_path;
	path_t *input_path;
} context_t;

static const embedded_asset_t EMBEDDED_ASSETS[] = {
	{"/refresh.svg", refresh_svg, sizeof(refresh_svg), "Content-Type: image/svg+xml\r\n"},
	{"/back.svg", back_svg, sizeof(back_svg), "Content-Type: image/svg+xml\r\n"},
	{"/style.css", style_css, sizeof(style_css), "Content-Type: text/css\r\n"},
	{"/script.js", script_js, sizeof(script_js), "Content-Type: application/javascript\r\n"},
};

#define EMBEDDED_ASSET_COUNT \
    (sizeof(EMBEDDED_ASSETS) / sizeof(EMBEDDED_ASSETS[0]))

static const embedded_asset_t *get_embedded_asset(const char *uri) {
	for (size_t i = 0; i < EMBEDDED_ASSET_COUNT; i++) {
		if (strcmp(uri, EMBEDDED_ASSETS[i].uri) == 0) return &EMBEDDED_ASSETS[i];
	}
	return NULL;
}

/*
each PDF and image file is a folder in the cache
these are refered to as "doc folders"
the images and PDF pages (converted to images) are stored inside doc folders
*/

static void event_handler(struct mg_connection *connection, int event, void *event_data) {
	if (event != MG_EV_HTTP_MSG) return;
	struct mg_http_message *http_msg = (struct mg_http_message *) event_data;
	context_t *ctx = (context_t *)connection->fn_data;

	arena_t scratch;
	arena_init(&scratch, 1*1000*1024);

	// INDEX
	if (mg_match(http_msg->uri, mg_str("/"), NULL)) {
		char refresh[16] = "";
		mg_http_get_var(&http_msg->query, "refresh", refresh, sizeof(refresh));
		if (strcmp(refresh, "true") == 0) {
			clear_dir(ctx->cache_path, &scratch); // WARNING:
			cache_copy_structure(&scratch, ctx->input_path, ctx->cache_path);
			cache_render_thumbnails(&scratch, ctx->input_path, ctx->cache_path, true);
			cache_create_full_render(&scratch, ctx->input_path, ctx->cache_path);
		}

		pathlist_t cache_dir_list;
		pathlist_init(&cache_dir_list, &scratch, ctx->cache_path->full_path, PATHLIST_DIRS_ONLY);
		pathlist_filter_by_ext(&cache_dir_list, NO_EXT, 1);

		// html

		char placeholder[1024 * cache_dir_list.count];
		placeholder[0] = '\0';
		
		for (int i = 0; i < cache_dir_list.count; i++) {
			path_t *cache_dir = cache_dir_list.items[i];
			size_t name_len = strlen(cache_dir->name);
			char button[256 + (name_len * 3)];

			int written = snprintf(
				button, sizeof(button),
				"<button onclick=\"loadFolder(id)\" id=\"%s\" class=\"folder-button\" name=\"%s\">\n"
				"  <h3>%s</h3>\n"
				"</button>\n",
				cache_dir->name, cache_dir->name, cache_dir->name);

			if (written < 0 || written >= sizeof(button)) continue;

			size_t used = strlen(placeholder);
			size_t remaining = sizeof(placeholder) - used - 1;
			if (written > remaining) break;
			snprintf(placeholder + used, remaining + 1, "%s", button);
		}

		char raw_html[index_html_len + 1];
		snprintf(raw_html, sizeof(raw_html), "%.*s", index_html_len, index_html);
		const char *placeholder_start = find_string(raw_html, PLACEHOLDER_STR, 1);
		if (!placeholder_start) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "Could not find the placeholder");
			arena_free(&scratch);
			return;
		}

		size_t placeholder_len = strlen(placeholder);
		size_t prefix_len = placeholder_start - raw_html;
		size_t suffix_len = sizeof(raw_html) - prefix_len - PLACEHOLDER_STR_LEN;

		char html[prefix_len + placeholder_len + suffix_len + 1];

		memcpy(html, index_html, prefix_len);
		memcpy(html + prefix_len, placeholder, placeholder_len);
		memcpy(html + prefix_len + placeholder_len, placeholder_start + PLACEHOLDER_STR_LEN, suffix_len);

		html[prefix_len + placeholder_len + suffix_len] = '\0';

		mg_http_reply(connection, 200, "Content-Type: text/html\r\n", "%s", html);
	}
	// FOLDER
	else if (mg_match(http_msg->uri, mg_str("/folder"), NULL)) {
		char var[MAX_NAME_LEN] = "";
		size_t var_len = mg_http_get_var(&http_msg->query, "name", var, sizeof(var));
		if (var_len >= sizeof(var) - 1) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "Folder name is too long");
			arena_free(&scratch);
			return;
		}
		char folder_name[MAX_NAME_LEN] = "";
		int folder_name_len = mg_url_decode(var, var_len, folder_name, sizeof(folder_name), 0);
		if (folder_name_len < 0) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "URL encoding invalid");
			arena_free(&scratch);
			return;
		}
		path_t folder_path;
		path_init(&folder_path, &scratch, "", ctx->cache_path->full_path, folder_name);
		
		// refresh
	
		path_t input_folder;
		path_init(&input_folder, &scratch, "", ctx->input_path->full_path, folder_name);
		char refresh[16] = "";
		mg_http_get_var(&http_msg->query, "refresh", refresh, sizeof(refresh));
		if (strcmp(refresh, "true") == 0) {
			clear_dir(&folder_path, &scratch); // WARNING:
			cache_copy_structure(&scratch, &input_folder, &folder_path);
			cache_render_thumbnails(&scratch, &input_folder, &folder_path, false);
			cache_create_full_render(&scratch, &input_folder, &folder_path);
		}

		path_t thumbnail_dir;
		path_init(&thumbnail_dir, &scratch, "", folder_path.full_path, THUMBNAIL_FOLDER_NAME);

		// TODO: custom function that returns thumbnail folder + the doc folder
		// instead of doing it in the FOR below

		pathlist_t thumbnail_list;
		pathlist_init(&thumbnail_list, &scratch, thumbnail_dir.full_path, PATHLIST_FILES_ONLY);

		size_t placeholder_size = (thumbnail_list.count > 0) ? 1024 * thumbnail_list.count : 1;
		char placeholder[placeholder_size];
		placeholder[0] = '\0';
		
		for (int i = 0; i < thumbnail_list.count; i++) {
			path_t *thumbnail = thumbnail_list.items[i];
			if (!path_exists(thumbnail)) continue;

			// TODO: multi page
			/*
	
			// has multiple pages?
			bool multi_page = false;
			
			path_t full_render_folder;
			path_init(&full_render_folder, &scratch, "", cache_folder.full_path, thumbnail->name);

			printf("%s\n", full_render_folder.full_path);
			if (!path_exists(&full_render_folder) || !is_dir(full_render_folder.full_path)) continue;
			pathlist_t full_render_folder_list;
			pathlist_init(&full_render_folder_list, &scratch, full_render_folder.full_path, 0);

			if (full_render_folder_list.count > 1) multi_page = true;

			const char *multi_page_html = multi_page ? "<div class=\"multi-page-indicator\"></div>" : "";
			*/

			// create doc folder path

			char doc_folder_name[MAX_PATH_LEN] = "";

			const char* thumbnail_name = find_last_char(thumbnail->full_path, '/', thumbnail->len, 1);
			if (!thumbnail_name) {
				mg_http_reply(connection, 302, "Location: /error.html\r\n", "Failed to generate doc folder path");
				arena_free(&scratch);
				return;
			}
			// remove extra .jpeg from end (appended when generating thumbnails)
			const char* thumbnail_jpeg_ext = find_last_char(thumbnail->full_path, '.', thumbnail->len, 0);
			if (!thumbnail_name) {
				mg_http_reply(connection, 302, "Location: /error.html\r\n", "Failed to generate doc folder path");
				arena_free(&scratch);
				return;
			}
			size_t doc_folder_name_len = thumbnail_jpeg_ext - thumbnail_name;
			memcpy(doc_folder_name, thumbnail_name, doc_folder_name_len);
			doc_folder_name[doc_folder_name_len] = '\0';

			path_t doc_folder;
			path_init(&doc_folder, &scratch, "", folder_path.full_path, doc_folder_name);

			// thumbnail stem
			
			char thumbnail_stem[MAX_PATH_LEN] = "";
			const char* thumnail_ext = find_string_reverse(thumbnail->name, ".", 2);
			size_t stem_len = thumnail_ext - thumbnail->name;
			memcpy(thumbnail_stem, thumbnail->name, stem_len);
			thumbnail_stem[stem_len] = '\0';

			// html
			
			const char *multi_page_html = "";

			char button[256 + (thumbnail->len * 2) + (stem_len * 2)];
			int written = snprintf(button, sizeof(button),
				"<div class=\"file-container\">"
				"  %s\n"
				"  <button onclick=\"loadFile(id)\" id=\"%s\" class=\"file-button\" name=\"%s\">\n"
				"    <img src=\"/image?name=%s\" class=\"thumbnail\"/>"
				"  </button>\n"
				"  <p>%s</p>\n"
				"</div>",
				multi_page_html, doc_folder.full_path, thumbnail_stem, thumbnail->full_path, thumbnail_stem);

			if (written < 0 || written >= sizeof(button)) continue;

			size_t used = strlen(placeholder);
			size_t remaining = sizeof(placeholder) - used - 1;
			if (written > remaining) break;
			snprintf(placeholder + used, remaining + 1, "%s", button);
		}
		char raw_html[folder_html_len + 1];
		snprintf(raw_html, sizeof(raw_html), "%.*s", folder_html_len, folder_html);
		const char *placeholder_start = find_string(raw_html, PLACEHOLDER_STR, 1);
		if (!placeholder_start) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "Could not find the placeholder");
			arena_free(&scratch);
			return;
		}

		size_t placeholder_len = strlen(placeholder);
		size_t prefix_len = placeholder_start - raw_html;
		size_t suffix_len = sizeof(raw_html) - prefix_len - PLACEHOLDER_STR_LEN;

		char html[prefix_len + placeholder_len + suffix_len + 1];

		memcpy(html, folder_html, prefix_len);
		memcpy(html + prefix_len, placeholder, placeholder_len);
		memcpy(html + prefix_len + placeholder_len, placeholder_start + PLACEHOLDER_STR_LEN, suffix_len);

		html[prefix_len + placeholder_len + suffix_len] = '\0';

		mg_http_reply(connection, 200, "Content-Type: text/html\r\n", "%s", html);
	}
	// FILE
	else if (mg_match(http_msg->uri, mg_str("/file"), NULL)) {
		// get vars from html
		char folder_var[MAX_NAME_LEN] = "";
		size_t folder_var_len = mg_http_get_var(&http_msg->query, "folder", folder_var, sizeof(folder_var));
		if (folder_var_len >= sizeof(folder_var) - 1) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "Folder name is too long");
			arena_free(&scratch);
			return;
		}
		char folder_name[MAX_NAME_LEN] = "";
		int folder_name_len = mg_url_decode(folder_var, folder_var_len, folder_name, sizeof(folder_name), 0);
		if (folder_name_len < 0) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "URL encoding invalid");
			arena_free(&scratch);
			return;
		}

		char file_var[MAX_NAME_LEN] = "";
		size_t file_var_len = mg_http_get_var(&http_msg->query, "name", file_var, sizeof(file_var));
		if (file_var_len >= sizeof(file_var) - 1) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "Folder name is too long");
			arena_free(&scratch);
			return;
		}
		char file_path[MAX_NAME_LEN] = "";
		int file_name_len = mg_url_decode(file_var, file_var_len, file_path, sizeof(file_path), 0);
		if (file_name_len < 0) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "URL encoding invalid");
			arena_free(&scratch);
			return;
		}

		path_t doc_folder;
		path_init(&doc_folder, &scratch, "", file_path);

		pathlist_t page_list;
		pathlist_init(&page_list, &scratch, doc_folder.full_path, PATHLIST_FILES_ONLY);

		size_t placeholder_size = (page_list.count > 0) ? 1024 * page_list.count : 1;
		char placeholder[placeholder_size];
		placeholder[0] = '\0';

		for (int i = 0; i < page_list.count; i++) {
			path_t *page = page_list.items[i];
			if (!path_exists(page)) continue;

			// html
			
			char page_image[256 + (page->len)];
			int written = snprintf(page_image, sizeof(page_image),
				"<img src=\"/image?name=%s\" class=\"page\"/>",
				page->full_path);

			if (written < 0 || written >= sizeof(page_image)) continue;

			// TODO: function?
			size_t used = strlen(placeholder);
			size_t remaining = sizeof(placeholder) - used - 1;

			if (written > remaining) break;
			snprintf(placeholder + used, remaining + 1, "%s", page_image);
			printf("\n%s\n", placeholder);
		}
		// TODO: function?
		char raw_html[file_html_len + 1];
		snprintf(raw_html, sizeof(raw_html), "%.*s", file_html_len, file_html);
		const char *placeholder_start = find_string(raw_html, PLACEHOLDER_STR, 1);
		if (!placeholder_start) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "Could not find the placeholder");
			arena_free(&scratch);
			return;
		}

		size_t placeholder_len = strlen(placeholder);
		size_t prefix_len = placeholder_start - raw_html;
		size_t suffix_len = sizeof(raw_html) - prefix_len - PLACEHOLDER_STR_LEN;

		char html[prefix_len + placeholder_len + suffix_len + 1];

		memcpy(html, file_html, prefix_len);
		memcpy(html + prefix_len, placeholder, placeholder_len);
		memcpy(html + prefix_len + placeholder_len, placeholder_start + PLACEHOLDER_STR_LEN, suffix_len);

		html[prefix_len + placeholder_len + suffix_len] = '\0';

		mg_http_reply(connection, 200, "Content-Type: text/html\r\n", "%s", html);
	}
	// IMAGE
	else if (mg_match(http_msg->uri, mg_str("/image"), NULL)) {
		char var[MAX_NAME_LEN] = "";
		size_t var_len = mg_http_get_var(&http_msg->query, "name", var, sizeof(var));
		if (var_len >= sizeof(var) - 1) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "Folder name is too long");
			arena_free(&scratch);
			return;
		}
		char file_path[MAX_NAME_LEN] = "";
		int file_path_len = mg_url_decode(var, var_len, file_path, sizeof(file_path), 0);
		if (file_path_len < 0) {
			mg_http_reply(connection, 302, "Location: /error.html\r\n", "URL encoding invalid");
			arena_free(&scratch);
			return;
		}

		path_t file;
		path_init(&file, &scratch, "", file_path);
		if (!path_exists(&file)) {
			mg_http_reply(connection, 404, "", "File not found");
			arena_free(&scratch);
			return;
		}

		struct mg_http_serve_opts opts = {0};
		mg_http_serve_file(connection, http_msg, file_path, &opts);
        }
	// STATIC
	else {
		char uri[MAX_PATH_LEN];
		mg_url_decode(http_msg->uri.buf, (int)http_msg->uri.len, uri, sizeof(uri), 0);
		const embedded_asset_t *embedded_asset = get_embedded_asset(uri);

		if (embedded_asset == NULL) {
			mg_http_reply(
				connection, 404, "Content-Type: text/html\r\n", "%.*s", error_html_len, error_html);
		} else {
			mg_http_reply(
				connection, 200, embedded_asset->content_type,
				"%.*s", embedded_asset->size, embedded_asset->data);
		}
	}

	arena_free(&scratch);
}

void app_main() {
	arena_t scratch;
	arena_init(&scratch, 1*1000*1024);

	// CONFIG

	bool reset_config = false;
	char input_dir_str[MAX_PATH_LEN] = "";

	path_t config_path;
	if (setup_config(&config_path, &scratch) == ERR) {
		printf("Config could not be created");
		return;
	}

	path_t config_file;
	path_init(&config_file, &scratch, "", config_path.full_path, "config.json");
	if (!path_exists(&config_file)) {
		write_config(&config_file, DEFAULT_INPUT_DIR);
	}

	while (true) {
		reset_config = false;

		char *config_text;
		if (read_config(&config_file, &scratch, &config_text) == ERR
			|| get_config_input_dir(input_dir_str, config_text) == ERR
			|| strcmp(input_dir_str, "") == 0) {
			printf("Config file: %s\n", config_file.full_path);
			printf("Please set your input directory in the config, then press Enter to continue...\n");
			getchar();
			continue;
		}

		path_t input_dir;
		path_init(&input_dir, &scratch, "", input_dir_str);
		if (!path_exists(&input_dir)) {
			printf("Input directory \"%s\" does not exist.\n", input_dir_str);
			printf("Please set a valid input directory in the config, then press Enter to retry...\n");
			getchar();
			continue;
		}
		break;
	}

	char *config_text;
	read_config(&config_file, &scratch, &config_text);
	if (strnlen(config_text, MAX_CONFIG_LINE) <= 0 || get_config_input_dir(input_dir_str, config_text) == ERR) {
		reset_config = true;
	}
	path_t input_dir;
	path_init(&input_dir, &scratch, "", input_dir_str);
	if (!path_exists(&input_dir)) reset_config = true;
	
	if (reset_config == true) {
		printf("Config file created at: %s\n", config_file.full_path);
		printf("Please set your input directory in the config, then press Enter to continue...\n");
		getchar();
	}

	// CACHE
	
	path_t cache_path;
	if (setup_cache(&cache_path, &scratch) == ERR) {
		printf("Cache could not be created");
		return;
	}

	clear_dir(&cache_path, &scratch); // WARNING:

	// LAYER 1: create all sub-folders, including doc folders
	cache_copy_structure(&scratch, &input_dir, &cache_path);

	// TODO: make this run parallel with opening index v

	// LAYER 2: render thumbnails
	cache_render_thumbnails(&scratch, &input_dir, &cache_path, true);

	// LAYER 3: create full render
	cache_create_full_render(&scratch, &input_dir, &cache_path);

	// WEB
	
	context_t ctx = {
		.input_path = &input_dir,
		.cache_path = &cache_path,
	};

	struct mg_mgr mgr;
	struct mg_connection *connection;

	mg_mgr_init(&mgr);

	connection = mg_http_listen(&mgr, "http://0.0.0.0:8888", event_handler, &ctx);
	if (connection == NULL) {
		printf("Failed to create listener\n");
		return;
	}
	printf("Starting web server on port 8888\n");
	open_browser("http://localhost:8888");

	for (;;) {
		mg_mgr_poll(&mgr, 100);
	}

	mg_mgr_free(&mgr);

	//printf("%.1f/%.1f MB\n", (double)scratch.offset / 1000.0 / 1024.0, (double)scratch.capacity / 1000.0 / 1024.0);
	return;
}
