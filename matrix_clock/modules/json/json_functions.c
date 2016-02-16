/*
 * json_functions.c
 *
 * Created: 9/29/2015 5:34:14 PM
 *  Author: Administrator
 */ 

#include "json_functions.h"

#define JSON_TOKENS 20

void json_get_token(jsmntok_t *tokens, char *js, char *buffer, uint8_t buffer_size, uint8_t i) {
	int len;
	jsmntok_t key;

	key = tokens[i];
	len = key.end - key.start;
	char keyString[ len+1 ];
	if (len < buffer_size) {
		memcpy( keyString, &js[ key.start ], len );
		keyString[ len ] = '\0';
		
		strcpy(buffer,keyString);
	}
}

void json_print_token(jsmntok_t *tokens, char *js, uint8_t i) {
	int len;
	jsmntok_t key;

	key = tokens[i];
	len = key.end - key.start;
	char keyString[ len+1 ];
	memcpy( keyString, &js[ key.start ], len );
	keyString[ len ] = '\0';
	//printf( "Key[%d]: %s\n", i, keyString );
	printf("token[%d]: %s \r\n",i,keyString);
}

jsmntok_t * json_tokenise(char *js)
{
	jsmn_parser parser;
	jsmn_init(&parser);

	unsigned int n = JSON_TOKENS;
	jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * n);

	int ret = jsmn_parse(&parser, js, strlen(js), tokens, n);

	while (ret == JSMN_ERROR_NOMEM) {
		n = n * 2 + 1;
		tokens = realloc(tokens, sizeof(jsmntok_t) * n);
		ret = jsmn_parse(&parser, js, strlen(js), tokens, n);
	}

#ifdef DEBUG_ON
	if (ret == JSMN_ERROR_INVAL) {
		puts("DEBUG: jsmn_parse: invalid JSON string");
	}
	if (ret == JSMN_ERROR_PART) {
		puts("DEBUG:jsmn_parse: truncated JSON string");
	}
#endif
	return tokens;
}

bool json_token_streq(char *js, jsmntok_t *t, char *s)
{
	return (strncmp(js + t->start, s, t->end - t->start) == 0
	&& strlen(s) == (size_t) (t->end - t->start));
}

char * json_token_tostr(char *js, jsmntok_t *t)
{
	js[t->end] = '\0';
	return js + t->start;
}