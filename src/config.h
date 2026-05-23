#include <string.h>
#include <stdio.h>

#include "jsmn.h"
#include "path.h"
#include "util.h"

err_t json_equals(const char *json, jsmntok_t *token, const char *text) {
	if (token->type == JSMN_STRING && (int)strlen(text) == token->end - token->start &&
		strncmp(json + token->start, text, token->end - token->start) == 0) {
		return OK;
	}
	return ERR;
}

err_t get_config_input_dir(char *input_dir, char *json_text) {
	jsmn_parser parser;
	jsmntok_t tokens[32];

	jsmn_init(&parser);
	int token_count =  jsmn_parse(&parser, json_text, strlen(json_text), tokens, 32);
	if (token_count < 1 || tokens[0].type != JSMN_OBJECT) return ERR;

	if (json_equals(json_text, &tokens[1], "input_dir") == OK) {
		jsmntok_t *value_token = &tokens[2];
		size_t val_len = value_token->end - value_token->start;
		if (val_len >= MAX_PATH_LEN) return ERR;
		strncpy(input_dir, json_text + value_token->start, val_len);
		input_dir[val_len] = '\0';
		return OK;
	}
	return ERR;
}
