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


char *replace_placeholder(const char *source, const char *placeholder, const char *replacement);
char *get_web_path(char *argv0);
void serve_html(struct mg_connection *connection, char* path, const PlaceholderList *placeholders);
void serve_error(struct mg_connection *connection, char* web_path);