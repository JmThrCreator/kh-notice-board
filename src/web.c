#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs_utils.h"
#include "mongoose.h"


typedef struct {
    const char *key;
    const char *value;
} Placeholder;

typedef struct {
    const int count;
    Placeholder *items;
} PlaceholderList;


char *replace_placeholder(const char *source, const char *placeholder, const char *replacement) {
    size_t count = 0;
    const char *tmp = source;
    while ((tmp = strstr(tmp, placeholder)) != NULL) {
        count++;
        tmp += strlen(placeholder);
    }

    // allocate mem for new str
    size_t new_len = strlen(source) + count * (strlen(replacement) - strlen(placeholder));
    char *result = malloc(new_len + 1);
    if (!result) return NULL;

    char *dest = result;
    const char *src = source;

    while ((tmp = strstr(src, placeholder)) != NULL) {
        // copy section before placeholder
        size_t segment_len = tmp - src;
        memcpy(dest, src, segment_len);
        dest += segment_len;

        // copy placeholder
        memcpy(dest, replacement, strlen(replacement));
        dest += strlen(replacement);

        // move src pointer after placeholder
        src = tmp + strlen(placeholder);
    }

    // copy end of str
    strcpy(dest, src);

    return result;
}


char *get_web_path(char *argv0) {
    static char path[PATH_LENGTH];

    if (realpath(argv0, path) == NULL) {
        perror("realpath");
        return NULL;
    }

    char *dir = dirname(path);
    snprintf(path, sizeof(path), "%s/../web", dir);
    return path;
}

void serve_html(struct mg_connection *connection, char* path, const PlaceholderList *placeholders) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        mg_http_reply(connection, 500, "", "Failed to load index.html");
        return;
    }
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);

    char *html = malloc(filesize + 1);
    if (html == NULL) {
        mg_http_reply(connection, 500, "", "Memory allocation error");
        free(html);
        return;
    }
    fread(html, 1, filesize, fp);
    html[filesize] = '\0';
    fclose(fp);

    char *parsed_html = html;
    for (size_t i = 0; i < placeholders->count; i++) {
        char *new_html = replace_placeholder(parsed_html, placeholders->items[i].key, placeholders->items[i].value);
        if (parsed_html != html) {
            free(parsed_html);
        }
        if (new_html == NULL) {
            mg_http_reply(connection, 500, "", "Memory allocation error");
            free(html);
            return;
        }
        parsed_html = new_html;
    }

    mg_http_reply(connection, 200, "Content-Type: text/html\r\n", "%s", parsed_html);
    if (parsed_html != html) {
        free(parsed_html);
    }
    free(html);
}