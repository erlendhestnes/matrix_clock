/*
 * esp8266.h
 *
 * Created: 1/18/2015 1:42:39 PM
 *  Author: Administrator
 */ 

#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "global.h"

#define SSID        "Inteno_68"
#define PASS        "A26A411568"
//#define DST_IP	"184.106.153.149"
#define DST_IP      "api.thingspeak.com"
#define ADDRESS		"http://api.thingspeak.com/channels/23643/feed.json?key=B48BEBINSXKRJRIN"
//#define ADDRESS "/channels/23643/feed.json?key=B48BEBINSXKRJRIN"
#define BUFFER		100
#define RX_BUFFER	200

#define ESP_RST		PIN4_bm
#define CH_EN		PIN5_bm

typedef enum {
	ESP8266_ERROR,
	ESP8266_SUCCESS,
	ESP8266_TIMEOUT,
	ESP8266_CONNECT,
	ESP8266_LINKED,
	ESP8266_CLOSED,
	ESP8266_NONE
} esp8266_status_t;

void esp8266_on(void);
void esp8266_off(void);
void esp8266_reset(void);

esp8266_status_t esp8266_setup(void);
esp8266_status_t esp8266_setup_webserver(void);
esp8266_status_t esp8266_run_webserver(char *str);
esp8266_status_t esp8266_run_simple_webserver(char *str);

esp8266_status_t esp8266_join_ap(char *ssid, char *pass);
esp8266_status_t esp8266_connect(char *host, char *addr, char *json);
esp8266_status_t connectWiFi(void);

void esp8266_get_rx_buffer(char *str);
void esp8266_extract_json(char *str);
#endif