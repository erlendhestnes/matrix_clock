/*
 * json_functions.c
 *
 * Created: 9/29/2015 5:34:14 PM
 *  Author: Administrator
 */ 

#include "json_functions.h"

void json_get_token(jsmntok_t *tokens, char *js, char *buffer, uint8_t i) {
	int len;
	jsmntok_t key;

	key = tokens[i];
	len = key.end - key.start;
	char keyString[ len+1 ];
	memcpy( keyString, &js[ key.start ], len );
	keyString[ len ] = '\0';
	
	strcpy(buffer,keyString);
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