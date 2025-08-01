#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "fs_utils.h"
#include "mongoose.h"
#include "web.h"

#define INPUT_DIR "/Users/jmaggio/Documents/Code/kh-notice-board/example-input"


static void event_handler(struct mg_connection *connection, int event, void *event_data) {
    char web_path[PATH_LENGTH];
    strncpy(web_path, (char *)connection->fn_data, PATH_LENGTH);

    if (event == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) event_data;
        
        // INDEX
        if (hm->uri.len == 1 && strncmp(hm->uri.buf, "/", 1) == 0) {
            char cache_path[PATH_LENGTH];
	        setup_cache(cache_path);
            PathList *cache_dir_list = create_path_list(cache_path, true);

            char *buttons = malloc(512*cache_dir_list->count);
            if (!buttons) return;
            buttons[0] = '\0';

            for (int i = 0; i < cache_dir_list->count; i++) {
                char *cache_dir = cache_dir_list->paths[i].name;
                char button[512];
                snprintf(button, sizeof(button),
                    "<button onclick=\"showLoad(id)\" id=\"%s\" type=\"submit\" class=\"folder-button\" name=\"%s\">\n"
                    "  <h3>%s</h3>\n"
                    "</button>\n",
                    cache_dir, cache_dir, cache_dir);

                strcat(buttons, button);
            }

            PlaceholderList placeholders = {
                .count = 1,
                .items = (Placeholder[]) {
                    { "{{folders}}", buttons },
                }
            };
            char html_path[PATH_LENGTH];
			snprintf(html_path, sizeof(html_path), "%s/index.html", web_path);
            serve_html(connection, html_path, &placeholders);
        }
		/*
        else if (hm->uri.len == 4 && strncmp(hm->uri.buf, "/api", 4) == 0) {
            mg_http_reply(
				c, 200, "Content-Type: application/json\r\n", 
                "{\"message\": \"Hello from API\", \"status\": \"success\"}");
        }
		*/
        else {
            mg_http_reply(
				connection, 404, "Content-Type: text/html\r\n", 
                "<h1>404 Not Found</h1><p>Page not found.</p>");
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
    
    mg_mgr_free(&mgr);
	return 0;
}