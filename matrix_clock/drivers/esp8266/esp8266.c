/*
 * esp8266.c
 *
 * Created: 1/18/2015 1:42:28 PM
 *  Author: Administrator
 */ 
#include "esp8266.h"
#include "../ht1632c/ht1632c.h"
#include "../eeprom/eeprom.h"
#include "../rtc/rtc.h"
#include "../port/port.h"
#include "../../fatfs/ff.h"

volatile bool json_found = false;
volatile bool got_reply = false;

volatile static uint16_t rx_ptr = 0;
static char rx_buffer[RX_BUFFER];

char ip_address[19];
char telnet_cmd[50];

esp8266_status_t status;

static inline void esp8266_send_cmd(char *str, uint16_t timeout_ms) {
	status = ESP8266_NONE;
	uart_write_str(str);
	_delay_ms(timeout_ms);
}

void esp8266_on(void) {
	PORTD.DIRSET = CH_EN;
	PORTD.OUTSET = CH_EN;
	_delay_ms(100);
}

void esp8266_off(void) {
	PORTD.OUTCLR = CH_EN;
}

void esp8266_reset(void) {
	PORTD.OUTSET = ESP_RST;
	_delay_ms(100);
	PORTD.OUTCLR = ESP_RST;
	_delay_ms(100);
}

esp8266_status_t esp8266_setup(void) {
    
	//Reset module
	esp8266_send_cmd("AT+RST",2000);
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Show firmware version
	esp8266_send_cmd("AT+GMR",50);
	
	//Set Data Mode
	esp8266_send_cmd("AT+CIPMODE=0",50);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Single connection mode
	esp8266_send_cmd("AT+CIPMUX=0",50);
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Select STA mode
	esp8266_send_cmd("AT+CWMODE=1",50);
	
	return ESP8266_SUCCESS;
}

void esp8266_list_ap(void) {
	esp8266_send_cmd("AT+CWLAP",4000);
}

void esp8266_update(void) {
	esp8266_send_cmd("AT+CIPUPDATE",10000);
}

esp8266_status_t esp8266_join_ap(char *ssid, char *pass) {
	
	uint8_t timeout = 150;
	uint16_t cnt = 0;
	char cmd[100];
	
	strcpy(cmd,"AT+CWJAP=\"");
	strcat(cmd,ssid);
	strcat(cmd,"\",\"");
	strcat(cmd,pass);
	strcat(cmd,"\"");
	esp8266_send_cmd(cmd,100);
	
	while (status != ESP8266_SUCCESS)
	{
		_delay_ms(200);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			return status;
		}
	}
	
	esp8266_send_cmd("AT+CWJAP?",100);
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}

	return ESP8266_SUCCESS;
}

esp8266_status_t esp8266_get_json(char *host, char *addr, char *json, uint8_t json_length) {
	
	uint16_t timeout = 240;
	uint16_t cnt = 0;
	//Try to make this dynamic?
	char cmd[150];
	
	//Set up TCP connection to host
	strcpy(cmd,"AT+CIPSTART=\"TCP\",\"");
	strcat(cmd,host);
	strcat(cmd,"\",80");
	esp8266_send_cmd(cmd,500);
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Count number of bytes to send
	char *number_of_bytes;
	itoa(strlen(addr) + 25, number_of_bytes, 10);
	strcpy(cmd, "AT+CIPSEND=");
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r"); //needs to be here...
	esp8266_send_cmd(cmd,50);
	
	//Request data by sending a GET
	strcpy(cmd,"GET ");
	strcat(cmd,addr);
	strcat(cmd," HTTP/1.0\r\n");
	esp8266_send_cmd(cmd,50);
	
	while (status != ESP8266_SUCCESS)
	{
		esp8266_send_cmd("",250);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			break;
		}
	}
	
	//Waiting for all the data
	while (status != ESP8266_CLOSED)
	{
		_delay_ms(250);
		
		if (cnt++ > timeout) {
			return ESP8266_TIMEOUT;
		} else if (status == ESP8266_ERROR) {
			break;
		}
	}
	
	if (json_found) {
		strncpy(json,rx_buffer,json_length);
		memset(rx_buffer,0,RX_BUFFER);
		json_found = false;
		rx_ptr = 0;
	}
	
	esp8266_send_cmd("AT+CIPCLOSE",50);
	
	return ESP8266_SUCCESS;
}

esp8266_status_t esp8266_setup_webserver(bool telnet, bool ap) {
	
	//Reset module
	esp8266_send_cmd("AT+RST\r",1000);
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	
	//Select mode
	if (telnet) {
		esp8266_send_cmd("AT+CWMODE=3",100);
	} else if (ap) {
		esp8266_send_cmd("AT+CWMODE=2",100);
	} else {
		esp8266_send_cmd("AT+CWMODE=1",100);
	}
	
	if (status != ESP8266_SUCCESS) {
		return status;
	}
	/*
	if (ap)
	{
		esp8266_send_cmd("AT+CWSAP=\"SMART_CLOCK\",\"1234\",0,3",100);
	}
	*/
	if (!telnet) {
		//Join access point
		esp8266_join_ap(env_var.wifi_ssid,env_var.wifi_pswd);
	}
	
	//List ip addresses
	esp8266_send_cmd("AT+CIFSR", 100);
	
	//Show ip address to user
	display_print_scrolling_text(ip_address,false);
	
	//Configure multiple connections
	esp8266_send_cmd("AT+CIPMUX=1",100);
	
	//Start server
	if (telnet) {
		esp8266_send_cmd("AT+CIPSERVER=1,8888",100);
		display_print_scrolling_text("TELNET ON",false);	
	} else {
		esp8266_send_cmd("AT+CIPSERVER=1,80",100);	
	}
	
	return ESP8266_SUCCESS;
}

static inline void at_cipsend(char *str) {
	char number_of_bytes[5];
	char cmd[50];
	
	got_reply = false;
	
	itoa_simple(number_of_bytes,strlen(number_of_bytes));
	//itoa(strlen(str),number_of_bytes,10);
	strcpy(cmd, "AT+CIPSEND=0,");
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r"); //needs to be here

	esp8266_send_cmd(cmd,10);
	esp8266_send_cmd(str,10);
}

static inline void at_cipsend2(void) {
	char number_of_bytes[5];
	char cmd[50];
	
	FATFS FatFs;		// FatFs work area needed for each volume
	FIL Fil;			// File object needed for each open file
	UINT br;
	
	got_reply = false;
	
	f_mount(&FatFs, "", 0);
	
	char line[1024];
	memset(line,0,1024);
	
	if (f_open(&Fil, "comp.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		uint32_t filesize = f_size(&Fil);
		do 
		{ 
			f_read(&Fil,line,1024,&br);
			esp8266_send_cmd("AT+CIPSEND=0,1024\r",25);
			printf("%s",line);
			_delay_ms(25);
		} while (f_eof(&Fil) == 0);
		
		/*while(f_gets(line,sizeof(line),&Fil)) {
			//itoa_simple(number_of_bytes,strlen(line));
			itoa(strlen(line),number_of_bytes,10);
			strcpy(cmd, "AT+CIPSEND=0,");
			strcat(cmd,number_of_bytes);
			strcat(cmd,"\r"); //needs to be here
			esp8266_send_cmd(cmd,5);
			printf("%s",line);
			_delay_ms(20);	
		}*/
		
		f_close(&Fil);
	} else {
		uart_write_str("Could not access sd card...");
	}
	
	if (f_open(&Fil, "zepto.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {
		uint32_t filesize = f_size(&Fil);
		do 
		{ 
			f_read(&Fil,line,1024,&br);
			esp8266_send_cmd("AT+CIPSEND=0,1024\r",0);
			while(status != ESP8266_SUCCESS);
			status = ESP8266_NONE;
			printf("%s",line);
			while(status != ESP8266_SUCCESS);
		} while (f_eof(&Fil) == 0);
		
		f_close(&Fil);
	} else {
		uart_write_str("Could not access sd card...");
	}
}
/*
esp8266_status_t esp8266_telnet_server(void) 
{	
	//Todo - lookup would be faster here
	
	bool quit = false;
	
	while(!quit) 
	{	
		btn_status = btn_check_press();
		if (btn_status == BTN4)
		{
			quit = true;
		}
		
		if (status == ESP8266_CONNECT) 
		{
			status = ESP8266_NONE;
			memset(telnet_cmd, 0, 50);
			at_cipsend("******************************************************\r\n");
			at_cipsend("***Welcome to the LED Matrix Clock telnet interface***\r\n");
			at_cipsend("******************************************************\r\n\r\n");
			at_cipsend("Type \"help\" to view all available commands\r\n>");
		} 
		else if (strstr(telnet_cmd,"led")) 
		{
			if (strstr(telnet_cmd,"on")) 
			{
				memset(telnet_cmd, 0, 50);
				ht1632c_fill_screen();
				at_cipsend("Turning on LEDs\r\n>");
			} 
			else if (strstr(telnet_cmd,"off")) 
			{
				ht1632c_clear_screen();
				memset(telnet_cmd, 0, 50);
				at_cipsend("Turning of LEDs\r\n>");
			} 
			else if (strstr(telnet_cmd,"down")) 
			{
				ht1632c_fade(1);
				memset(telnet_cmd, 0, 50);
				at_cipsend("Dimming down LEDs\r\n>");
			} 
			else if (strstr(telnet_cmd,"up")) 
			{
				ht1632c_fade(15);
				memset(telnet_cmd, 0, 50);
				at_cipsend("Dimming up LEDs\r\n>");
			} 
			else 
			{
				memset(telnet_cmd, 0, 50);
				at_cipsend("Command not found \r\n>");
			}
		} 
		else if (strstr(telnet_cmd,"get")) 
		{
			if (strstr(telnet_cmd,"password")) 
			{
				memset(telnet_cmd, 0, 50);
				at_cipsend(PASS);
				at_cipsend("\r\n>");
			} 
			else if (strstr(telnet_cmd,"ssid")) 
			{
				memset(telnet_cmd, 0, 50);
				at_cipsend(SSID);
				at_cipsend("\r\n>");
			} 
			else if (strstr(telnet_cmd,"time")) 
			{
				char temp[5];
				
				memset(telnet_cmd, 0, 50);
				
				itoa(env_var.time.hours,temp,10);
				at_cipsend("Time - h:");
				at_cipsend(temp);
				itoa(env_var.time.minutes,temp,10);
				at_cipsend(" m:");
				at_cipsend(temp);
				itoa(env_var.time.seconds,temp,10);
				at_cipsend(" s:");
				at_cipsend(temp);
				
				itoa(env_var.time.day,temp,10);
				at_cipsend(" - Day:");
				at_cipsend(temp);
				itoa(env_var.time.week,temp,10);
				at_cipsend(" Week:");
				at_cipsend(temp);
				itoa(env_var.time.month,temp,10);
				at_cipsend(" Month:");
				at_cipsend(temp);
				itoa(env_var.time.year,temp,10);
				at_cipsend(" Year:");
				at_cipsend(temp);
				
				at_cipsend("\r\n>");
				
			} 
			else 
			{
				memset(telnet_cmd, 0, 50);
				at_cipsend("Command not found \r\n>");
			}
		} 
		else if (strstr(telnet_cmd,"set")) 
		{
			if (strstr(telnet_cmd,"time")) 
			{
				time_env_t time;
				uint16_t timeout = 1000;
				uint16_t cnt = 0;
				char *token;
				memset(telnet_cmd, 0, 50);
				at_cipsend("Date format - [hh:mm:ss:yyyy]\r\n>");
				
				while(!got_reply && (++cnt < timeout)) 
				{
					_delay_ms(60);
				}
				
				if (strlen(telnet_cmd) == 24) 
				{	
					token = strtok(telnet_cmd,":");
					if (token == NULL) 
					{
						at_cipsend("Wrong time format!\r\n>");
						return ESP8266_ERROR;
					}
					
					token = strtok(NULL,":");
					if (token == NULL) 
					{
						at_cipsend("Wrong time format...failed at hours!\r\n>");
						return ESP8266_ERROR;
					}
					
					time.hours = atoi(token);
					if (time.hours > 23) 
					{
						at_cipsend("Wrong time format...failed at hours!\r\n>");
						return ESP8266_ERROR;
					}
					
					token = strtok(NULL,":");
					if (token == NULL) 
					{
						at_cipsend("Wrong time format...failed at minutes!\r\n>");
						return ESP8266_ERROR;
					}
					
					time.minutes = atoi(token);
					if (time.minutes > 59) 
					{
						at_cipsend("Wrong time format...failed at minutes!\r\n>");
						return ESP8266_ERROR;
					}
					
					token = strtok(NULL,":");
					if (token == NULL) 
					{
						at_cipsend("Wrong time format...failed at seconds!\r\n>");
						return ESP8266_ERROR;
					}
					
					time.seconds = atoi(token);
					if (time.seconds > 59) 
					{
						at_cipsend("Wrong time format...failed at seconds!\r\n>");
						return ESP8266_ERROR;
					}
					
					token = strtok(NULL,":");
					if (token == NULL) 
					{
						at_cipsend("Wrong time format...failed at years!\r\n>");
						return ESP8266_ERROR;
					}
					time.year = atoi(token);
					
					//Store configuration in EEPROM
					EEPROM_WriteEnv();
					
					at_cipsend("Time was successfully set!\r\n>");
				} 
				else 
				{
					at_cipsend("Wrong time format or timeout!\r\n>");
				}
				
			} 
			else if (strstr(telnet_cmd,"ssid")) 
			{	
				uint16_t timeout = 1000;
				uint16_t cnt = 0;
				char *token;
				memset(telnet_cmd, 0, 50);
					
				at_cipsend("Enter SSID, max 20 characters\r\n>");
					
				while(!got_reply && (++cnt < timeout)) 
				{
					_delay_ms(60);
				}
					
				if (token == NULL) 
				{
					at_cipsend("Something went wrong!\r\n>");
					return ESP8266_ERROR;
				}
					
				token = strtok(NULL,":");
				
				strncpy(env_var.wifi_ssid,token,sizeof(env_var.wifi_ssid));
				
				//Store configuration in EEPROM
				EEPROM_WriteEnv();
					
				memset(telnet_cmd, 0, 50);
				at_cipsend("SSID set\r\n>");
			}
			else if (strstr(telnet_cmd,"password"))
			{
				uint16_t timeout = 1000;
				uint16_t cnt = 0;
				char *token;
				memset(telnet_cmd, 0, 50);
				
				at_cipsend("Enter PASSWORD, max 20 characters\r\n>");
				
				while(!got_reply && (++cnt < timeout))
				{
					_delay_ms(60);
				}
				
				if (token == NULL)
				{
					at_cipsend("Something went wrong!\r\n>");
					return ESP8266_ERROR;
				}
				
				token = strtok(NULL,":");
				
				strncpy(env_var.wifi_pswd,token,sizeof(env_var.wifi_pswd));
				
				//Store configuration in EEPROM
				EEPROM_WriteEnv();
				
				memset(telnet_cmd, 0, 50);
				at_cipsend("WiFi PASSWORD set\r\n>");
			} 
			else 
			{
				memset(telnet_cmd, 0, 50);
				at_cipsend("Not a valid command\r\n>");
			}
		} 
		else if (strstr(telnet_cmd,"wifi")) 
		{
			if (strstr(telnet_cmd,"off")) 
			{
				memset(telnet_cmd, 0, 50);
				at_cipsend("Turning off wifi.\r\n");
				esp8266_off();
				quit = true;
			} 
			else 
			{
				memset(telnet_cmd, 0, 50);
				at_cipsend("Command not found \r\n");
			}
		} 
		else if (strstr(telnet_cmd,"help")) 
		{
			memset(telnet_cmd, 0, 50);
			at_cipsend("-----------------------------------\r\n");
			at_cipsend("led on - Turns the entire LED array on.\r\n");
			at_cipsend("led off - Turns the entire LED array off.\r\n");
			at_cipsend("led down - Dim down LEDs.\r\n");
			at_cipsend("led up - Dim up LEDs.\r\n");
			at_cipsend("-----------------------------------\r\n");
			//TODO: Add day and month
			at_cipsend("set time - Sets the time and date of the clock. Format [hh:mm:ss:yyyy]\r\n");
			at_cipsend("set ssid - Sets the SSID for the WiFi connection \r\n");
			at_cipsend("set password - Sets the PASSWORD for the WiFi connection. \r\n");
			at_cipsend("-----------------------------------\r\n");
			at_cipsend("get time - Show the time and date of the clock. \r\n");
			at_cipsend("get ssid - Show the SSID for the WiFi connection \r\n");
			at_cipsend("get password - Show the PASSWORD for the WiFi connection. \r\n");
			at_cipsend("-----------------------------------\r\n");
			at_cipsend("wifi off - Turn off wifi and return to menu \r\n");
			at_cipsend("-----------------------------------\r\n>");
		}	
	}
}
*/
esp8266_status_t esp8266_run_simple_webserver(void) 
{
	if(rx_ptr > 100) // check if the ESP8266 is sending data
	//if (status = ESP8266_CONNECT)
	{
		_delay_ms(1000);
		at_cipsend2();
		_delay_ms(10000);
		esp8266_send_cmd("AT+CIPCLOSE=0\r", 100);
		while (1);
	}
}

ISR(USARTD0_RXC_vect) {
	
	char rx_temp = USARTD0.DATA;
	
	if (rx_temp == '\n') {
		if (strstr(rx_buffer,"OK") || strstr(rx_buffer,"ready")) {
			status = ESP8266_SUCCESS;
		} else if (strstr(rx_buffer,"ERROR") || strstr(rx_buffer,"FAIL")) {
			status = ESP8266_ERROR;
		} else if (strstr(rx_buffer,"CONNECT")) {
			status = ESP8266_CONNECT;
		} else if (strstr(rx_buffer,"CLOSED")) {
			status = ESP8266_CLOSED;
		} else if (strstr(rx_buffer,"192")) {
			strncpy(ip_address,strchr(rx_buffer,'\"'),19);
		//This might be error prone...
		} else if (strstr(rx_buffer,"+IPD")) {
			got_reply = true;
			strncpy(telnet_cmd,rx_buffer,50);
		}
		
		if (strstr(rx_buffer,"{")) {
			json_found = true;	
		} else {
			//Flush buffer
			rx_ptr = 0;
			memset(rx_buffer, 0, RX_BUFFER);	
		}
	} else {
		rx_buffer[rx_ptr++] = rx_temp;
		
		//Buffer overflow guard
		if (rx_ptr == RX_BUFFER) {
			rx_ptr = 0;
		}
	}
}