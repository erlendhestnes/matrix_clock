/*
 * esp8266.h
 *
 * Created: 1/18/2015 1:42:39 PM
 *  Author: Administrator
 */ 

#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "../../global.h"
#include "../../json/jsmn.h"

//Arkitekt Christies Gt 11
#define SSID        "Inteno_68"
#define PASS        "A26A411568"

//Lundaasen 12
//#define SSID        "Loqal_43"
//#define PASS        "892B9A1758"

//#define DST_IP	"184.106.153.149"
//#define DST_IP      "api.thingspeak.com"
//#define ADDRESS		"http://api.thingspeak.com/channels/23643/feed.json?key=B48BEBINSXKRJRIN"
//#define ADDRESS "/channels/23643/feed.json?key=B48BEBINSXKRJRIN"

#define TIME_IP "ukenummer.no"
#define TIME_ADDR "http://ukenummer.no/json"

#define WEATHER_IP "api.thingspeak.com"
#define WEATHER_ADDR "https://api.thingspeak.com/apps/thinghttp/send_request?api_key=6ZW7E1P3KSIPD8E2"

#define WEATHER_IP2 "api.openweathermap.org"
#define WEATHER_ADDR2 "http://api.openweathermap.org/data/2.5/weather?zip=7012,no&units=metric&appid=bd82977b86bf27fb59a04b61b657fb6f"

//EDIT:

#define RX_BUFFER	220

#define ESP_RST		PIN4_bm
#define CH_EN		PIN5_bm

#define SSID_EEPROM_PAGE	0

typedef enum {
	ESP8266_ERROR,
	ESP8266_SUCCESS,
	ESP8266_CIPSEND,
	ESP8266_TIMEOUT,
	ESP8266_CONNECT,
	ESP8266_LINKED,
	ESP8266_CLOSED,
	ESP8266_GET_REQ,
	ESP8266_POST_REQ,
	ESP8266_NONE
} esp8266_status_t;

void esp8266_on(void);
void esp8266_off(void);
void esp8266_reset(void);

esp8266_status_t esp8266_setup(void);
esp8266_status_t esp8266_setup_webserver(bool sta, bool ap);
esp8266_status_t esp8266_run_webserver(char *str);
esp8266_status_t esp8266_configure_ssid_and_password(void);
esp8266_status_t esp8266_telnet_server(void);

void esp8266_list_ap(void);
void esp8266_update(void);
esp8266_status_t esp8266_join_ap(char *ssid, char *pass);
esp8266_status_t esp8266_get_json(char *host, char *addr, char *json, uint8_t json_length);
esp8266_status_t connectWiFi(void);

void esp8266_get_rx_buffer(char *str);
void esp8266_extract_json(char *str);
#endif