#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "cache.h"
#include "fs_utils.h"
#include "mongoose.h"
#include "web.h"

#define INPUT_DIR "/Users/jmaggio/Documents/Code/kh-notice-board/example-input"
#define MAX_PAGES 300
const char *VALID_IMG_EXT[] = { "png", "jpg", "jpeg" };

static void event_handler(struct mg_connection *connection, int event, void *event_data) {
    char web_path[PATH_LENGTH];
    strncpy(web_path, (char *)connection->fn_data, PATH_LENGTH);

    if (event == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) event_data;

        // INDEX
        if (mg_match(hm->uri, mg_str("/"), NULL)) {
            // refresh
            char refresh[16] = "";
            mg_http_get_var(&hm->query, "refresh", refresh, sizeof(refresh));
            if (strcmp(refresh, "true") == 0) {
                update_cache(INPUT_DIR);
            }

            char cache_path[PATH_LENGTH];
	        setup_cache(cache_path);
            PathList *cache_dir_list = create_path_list(cache_path, true);

            // html

            char *html = malloc((512*cache_dir_list->count)+512);
            if (!html) {
                free(cache_dir_list);
                return;
            }
            html[0] = '\0';

            for (int i = 0; i < cache_dir_list->count; i++) {
                char *cache_dir = cache_dir_list->paths[i].name;
                char button[512];
                snprintf(button, sizeof(button),
                    "<button onclick=\"loadFolder(id)\" id=\"%s\" class=\"folder-button\" name=\"%s\">\n"
                    "  <h3>%s</h3>\n"
                    "</button>\n",
                    cache_dir, cache_dir, cache_dir);

                strcat(html, button);
            }

            PlaceholderList placeholders = {
                .count = 1,
                .items = (Placeholder[]) {
                    { "{{ placeholder }}", html },
                }
            };
            char html_path[PATH_LENGTH];
			snprintf(html_path, sizeof(html_path), "%s/index.html", web_path);
            serve_html(connection, html_path, &placeholders);
            free(html);
            free_path_list(cache_dir_list);
        }
        // FOLDERS
        else if (mg_match(hm->uri, mg_str("/folder"), NULL)) {
            // folder name
            char var[SUFFIX_LENGTH] = "";
            mg_http_get_var(&hm->query, "name", var, sizeof(var));
            char folder_name[SUFFIX_LENGTH] = "";
            mg_url_decode(var, sizeof(var), folder_name, sizeof(folder_name), 0);

            char cache_path[PATH_LENGTH];
	        setup_cache(cache_path);

            // refresh
            char refresh[16] = "";
            mg_http_get_var(&hm->query, "refresh", refresh, sizeof(refresh));
            if (strcmp(refresh, "true") == 0) {
                update_cache(INPUT_DIR);
            }

            // folder list
            char folder_path[PATH_LENGTH];
            snprintf(folder_path, sizeof(folder_path), "%s/%s", cache_path, folder_name);
            struct stat st;
            if (stat(folder_path, &st) != 0) {
                serve_error(connection, web_path);
                return;
            }
            PathList *folder_dir_list = create_path_list(folder_path, false);

            char *html = malloc(512*folder_dir_list->count);
            if (!html) {
                free_path_list(folder_dir_list);
                return;
            }
            html[0] = '\0';

            for (int i = 0; i < folder_dir_list->count; i++) {
                char *file_name = folder_dir_list->paths[i].name;
                char *file_path = folder_dir_list->paths[i].path;

                // cache_path/folder/file_name/0.png - first page
                char thumbnail_path[PATH_LENGTH];
                int valid = 0;
                for (int i = 0; i < sizeof(VALID_IMG_EXT) / sizeof(VALID_IMG_EXT[0]); i++) {
                    snprintf(thumbnail_path, sizeof(thumbnail_path), "%s/0.%s", file_path, VALID_IMG_EXT[i]);
                    if (access(thumbnail_path, F_OK) == 0) {
                        valid = 1;
                        break;
                    }
                }
                if (!valid) {
                    free(html);
                    free_path_list(folder_dir_list);
                    serve_error(connection, web_path);
                    return;
                }
                
                // indicates more than 1 page
                const bool multi_page = has_multi_files(folder_dir_list->paths[i].path);
                char multi_page_html[64] = "";
                if (multi_page) {
                    strcpy(multi_page_html, "<div class=\"multi-page-indicator\"></div>");
                }

                char button[1024]; 
                snprintf(button, sizeof(button),
                    "<div class=\"file-container\">"
                    "  %s\n"
                    "  <button onclick=\"loadFile(id)\" id=\"%s\" class=\"file-button\" name=\"%s\">\n"
                    "    <img src=\"/image?name=%s\" class=\"thumbnail\"/>"
                    "  </button>\n"
                    "  <p>%s</p>\n"
                    "</div>",
                    multi_page_html, file_path, file_name, thumbnail_path, file_name);
                strcat(html, button);
            }

            PlaceholderList placeholders = {
                .count = 1,
                .items = (Placeholder[]) {
                    { "{{ placeholder }}", html },
                }
            };
            char html_path[PATH_LENGTH];
			snprintf(html_path, sizeof(html_path), "%s/folder.html", web_path);
            serve_html(connection, html_path, &placeholders);
            free(html);
            free_path_list(folder_dir_list);
        }

        // FILE
        else if (mg_match(hm->uri, mg_str("/file"), NULL)) {
            char var[SUFFIX_LENGTH] = "";
            mg_http_get_var(&hm->query, "name", var, sizeof(var));

            char file_path[SUFFIX_LENGTH] = "";
            mg_url_decode(var, sizeof(var), file_path, sizeof(file_path), 0);

            char* html = malloc(64);
            if (!html) return;
            html[0] = '\0';

            for (int i = 0; i < MAX_PAGES; i++) {
                char page_path[PATH_LENGTH];
                bool valid = false;
                for (int j = 0; j < sizeof(VALID_IMG_EXT) / sizeof(VALID_IMG_EXT[0]); j++) {
                    snprintf(page_path, sizeof(page_path), "%s/%d.%s", file_path, i, VALID_IMG_EXT[j]);
                    if (access(page_path, F_OK) == 0) {
                        valid = true;
                        break;
                    }
                }
                if (!valid) break;

                char buffer[512];
                snprintf(buffer, sizeof(buffer),
                    "<img src=\"/image?name=%s\" class=\"page\"/>",
                    page_path);

                size_t new_len = strlen(html) + strlen(buffer) + 1;
                char* tmp = realloc(html, new_len);
                if (tmp == NULL) {
                    free(html);
                    return;
                }
                html = tmp;
                strcat(html, buffer);
            }

            PlaceholderList placeholders = {
                .count = 1,
                .items = (Placeholder[]) {
                    { "{{ placeholder }}", html },
                }
            };
            char html_path[PATH_LENGTH];
			snprintf(html_path, sizeof(html_path), "%s/file.html", web_path);
            serve_html(connection, html_path, &placeholders);
            free(html);
        }

        // IMAGE
        else if (mg_match(hm->uri, mg_str("/image"), NULL)) {
            char var[SUFFIX_LENGTH] = "";
            mg_http_get_var(&hm->query, "name", var, sizeof(var));

            char file_path[SUFFIX_LENGTH] = "";
            mg_url_decode(var, sizeof(var), file_path, sizeof(file_path), 0);

            char cache_path[PATH_LENGTH];
	        setup_cache(cache_path);

            if (access(file_path, F_OK) != 0) {
                mg_http_reply(connection, 404, "", "File not found");
                return;
            }

            struct mg_http_serve_opts opts = {0};
            mg_http_serve_file(connection, hm, file_path, &opts);
        }

        else {
            // STATIC
            char uri[PATH_LENGTH];
            mg_url_decode(hm->uri.buf, (int)hm->uri.len, uri, sizeof(uri), 0);

            char path[PATH_LENGTH];
            snprintf(path, sizeof(path), "%s/%s", web_path, uri);

            struct stat st;
            if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
                struct mg_http_serve_opts opts = {0};
                mg_http_serve_file(connection, hm, path, &opts);
            } else {
                mg_http_reply(
				connection, 404, "Content-Type: text/html\r\n", 
                "<h1>404 Not Found</h1><p>Page not found.</p>");
            }
        }
    }
}

int main(int argc, char **argv) {
    // update_cache(INPUT_DIR);

    char *web_path = get_web_path(argv[0]);
    if (!web_path) {
        fprintf(stderr, "Failed to resolve index.html path\n");
        return 1;
    }

    struct mg_mgr mgr;
    struct mg_connection *connection;
    
    // init event manager
    mg_mgr_init(&mgr);
    
    connection = mg_http_listen(&mgr, "http://0.0.0.0:8000", event_handler, web_path);
    if (connection == NULL) {
        printf("Failed to create listener\n");
        return 1;
    }
    printf("Starting web server on port 8000 - http://localhost:8000\n");
    
    // event loop
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    
    free(web_path);
    mg_mgr_free(&mgr);
	return 0;
}