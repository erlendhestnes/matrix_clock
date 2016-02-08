/*
 * esp8266.h
 *
 * Created: 1/18/2015 1:42:39 PM
 *  Author: Administrator
 */ 

#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "../../global.h"
#include "../../modules/json/jsmn.h"

static volatile bool wdt_triggered = false;

//Arkitekt Christies Gt 11
#define SSID        "Inteno_68"
#define PASS        "A26A411568"

//Lundaasen 12
//#define SSID        "Loqal_43"
//#define PASS        "892B9A1758"

#define WEATHER_IP "www.grenaderen.no"
#define WEATHER_ADDR "http://grenaderen.no/test/weather.php"

#define TIME_IP "www.grenaderen.no"
#define TIME_ADDR "http://grenaderen.no/test/ntp.php"

#define RX_BUFFER	250

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
void esp8266_deep_sleep(void);
void esp8266_update(void);

esp8266_status_t esp8266_setup(void);
esp8266_status_t esp8266_setup_webserver(bool sta, bool ap);
esp8266_status_t esp8266_configure_ssid_and_password(void);

void esp8266_list_ap(void);

esp8266_status_t esp8266_join_ap(char *ssid, char *pass);
esp8266_status_t esp8266_get_json(char *host, char *addr, char *json, uint8_t json_length);

#endif