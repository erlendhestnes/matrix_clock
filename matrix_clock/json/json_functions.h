/*
 * json_functions.h
 *
 * Created: 9/29/2015 5:44:03 PM
 *  Author: Administrator
 */ 


#ifndef JSON_FUNCTIONS_H_
#define JSON_FUNCTIONS_H_

#include "../global.h"
#include "jsmn.h"

void json_get_token(jsmntok_t *tokens, char *js, char *buffer, uint8_t i);
void json_print_token(jsmntok_t *tokens, char *js, uint8_t i);

#endif /* JSON_FUNCTIONS_H_ */