/*
 * esp8266.h
 *
 * Created: 1/18/2015 1:42:39 PM
 *  Author: Administrator
 */ 

#ifndef _ESP8266_H_
#define _ESP8266_H_

#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
	ERROR,
	SUCCESS	
} esp8266_status_t;

#define SSID        "Inteno_68"
#define PASS        "A26A411568"
#define DST_IP		"184.106.153.149"
//#define DST_IP      "api.thingspeak.com"
//#define ADDRESS "http://api.thingspeak.com/channels/23643/feed.json?key=B48BEBINSXKRJRIN"
#define ADDRESS "/channels/23643/feed.json?key=B48BEBINSXKRJRIN"
#define BUFFER 100
#define RX_BUFFER 200

#define RST   PIN4_bm
#define CH_EN PIN5_bm

esp8266_status_t esp8266_init(void);
void esp8266_on(void);
void esp8266_off(void);
void esp8266_connect(void);
esp8266_status_t connectWiFi(void);

#endif