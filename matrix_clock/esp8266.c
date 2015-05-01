/*
 * esp8266.c
 *
 * Created: 1/18/2015 1:42:28 PM
 *  Author: Administrator
 */ 
#include "esp8266.h"
#include "ht1632c.h"

char cmd[BUFFER];
char html[10];

volatile static uint16_t rx_ptr = 0;
static char rx_buffer[RX_BUFFER];

const bool printReply = true;
const char line[] = "-----\n\r";
int loopCount=0;

char command[20];

char ipAddress [20];
char name[30];
int lenHtml = 0;
char temp[5];

static inline void flush_rx_buffer(void) {
	rx_ptr = 0;
	memset(rx_buffer, 0, RX_BUFFER);
}

static inline void flush_cmd_buffer(void) {
	memset(cmd, 0, BUFFER);
}

static inline void esp8266_send_cmd(char *str, uint16_t timeout_ms) {
	flush_rx_buffer();
	//puts(str);
	printf("%s\r\n",str);
	_delay_ms(timeout_ms);
}

void esp8266_on(void) {
	PORTD.DIRSET = CH_EN;
	PORTD.OUTSET = CH_EN;
	_delay_ms(3000);
}

void esp8266_off(void) {
	PORTD.OUTCLR = CH_EN;
}

void esp8266_reset(void) {
	PORTD.OUTSET = ESP_RST; //| PIN5_bm;_
	_delay_ms(100);
	PORTD.OUTCLR = ESP_RST;
	_delay_ms(100);
}

esp8266_status_t esp8266_setup(void) {
    
	//Reset module
	esp8266_send_cmd("AT+RST\r",5000);
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Set Data Mode
	esp8266_send_cmd("AT+CIPMODE=0",1000);
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Single connection mode
	esp8266_send_cmd("AT+CIPMUX=0",1000);
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Select mode
	esp8266_send_cmd("AT+CWMODE=1",1000);
	
	return SUCCESS;
}

esp8266_status_t esp8266_join_ap(char *ssid, char *pass) {
	flush_cmd_buffer();
	strcat(cmd,"AT+CWJAP=\"");
	strcat(cmd,ssid);
	strcat(cmd,"\",\"");
	strcat(cmd,pass);
	strcat(cmd,"\"");
	esp8266_send_cmd(cmd,5000);
	esp8266_send_cmd("AT+CWJAP\r",5000);
	
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}

	return SUCCESS;
}

esp8266_status_t esp8266_connect(char *host, char *addr) {
	//Set up TCP connection to host
	flush_cmd_buffer();
	strcat(cmd,"AT+CIPSTART=\"TCP\",\"");
	strcat(cmd,host);
	strcat(cmd,"\",80");
	esp8266_send_cmd(cmd,2000);
	
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Count number of bytes to send
	flush_cmd_buffer();
	char *number_of_bytes;
	sprintf(number_of_bytes, "%d", strlen(addr)+20);
	strcat(cmd, "AT+CIPSEND=");
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r"); //needs to be here
	esp8266_send_cmd(cmd,1000);
	
	//Send data
	flush_cmd_buffer();
	strcat(cmd,"GET ");
	strcat(cmd,addr);
	strcat(cmd," HTTP/1.0");
	esp8266_send_cmd(cmd,1000);
	
	uint16_t timeout = 40;
	uint16_t cnt = 0;
	
	while (strstr(rx_buffer,"Unlink") == NULL)
	{
		esp8266_send_cmd("",250);
		
		if (cnt++ > timeout) {
			return TIMEOUT;
		}
	}
	_delay_ms(5000);
	/*
	cnt = 0;
	while (strstr(rx_buffer,"Unlink") == NULL)
	{
		_delay_ms(250);
		if (cnt++ > timeout) {
			return TIMEOUT;
		}
	}*/
	
	return SUCCESS;
}

static inline void remove_substring(char *src, char *sub) {
	char *p;
	if ((p=strstr(src,sub)) != NULL) {
		memmove(p,p+strlen(sub), strlen(p+strlen(sub))+1);
		// alternative
		// strcpy(p,p+strlen(sub));
	}
}

void esp8266_get_rx_buffer(char *str) {
	
	strncpy(str,rx_buffer,strlen(rx_buffer));
}

void esp8266_extract_json(char *str) {
	
	char *tmp;
	char *tmp2;
	
	tmp = strchr(rx_buffer,'{');
	tmp2 = strrchr(tmp,'}');
	*tmp2++;
	
	remove_substring(tmp,tmp2);
	
	strncpy(str,tmp,strlen(tmp));
}

esp8266_status_t esp8266_setup_webserver(void) {
	
	//Reset module
	esp8266_send_cmd("AT+RST\r",3000);
	if (strstr(rx_buffer,"OK") == NULL) {
		return ERROR;
	}
	
	//Select mode
	esp8266_send_cmd("AT+CWMODE=1",500);
	
	//Join access point
	esp8266_join_ap(SSID,PASS);
	
	esp8266_send_cmd("AT+CIFSR\r", 500);
	
	// parse ip address.
	int len = strlen(rx_buffer);
	bool done=false;
	bool error = false;
	int pos = 0;
	while (!done)
	{
		if ( rx_buffer[pos] == 10) { done = true;}
		pos++;
		if (pos > len) { done = true;  error = true;}
	}
	
	if (!error)
	{
		int buffpos = 0;
		done = false;
		while (!done)
		{
			if ( rx_buffer[pos] == 13 ) { done = true; }
			else { ipAddress[buffpos] = rx_buffer[pos];    buffpos++; pos++;   }
		}
		ipAddress[buffpos] = 0;
	}
	else { strcpy(ipAddress,"ERROR"); }
	
	//Configure multiple connections
	esp8266_send_cmd("AT+CIPMUX=1",500);
	
	//Start server
	esp8266_send_cmd("AT+CIPSERVER=1,80",500);	
	
	return SUCCESS;
}

static inline void at_cipsend(char *str) {
	char *number_of_bytes;
	
	//strcpy(html,str);
	
	//flush_cmd_buffer();
	sprintf(number_of_bytes, "%d", strlen(str));
	strcpy(cmd, "AT+CIPSEND=0,");
	strcat(cmd,number_of_bytes);
	strcat(cmd,"\r"); //needs to be here

	esp8266_send_cmd(cmd,2000);
	esp8266_send_cmd(str,2000);
}

esp8266_status_t esp8266_run_simple_webserver(char *str) 
{
	if(rx_ptr > 190) // check if the ESP8266 is sending data
	{
		_delay_ms(1000);
		at_cipsend(str);
		_delay_ms(10000);
		esp8266_send_cmd("AT+CIPCLOSE=0\r", 100);
	}
}

esp8266_status_t esp8266_run_webserver(char *str) 
{
 if(rx_ptr > 190) // check if the ESP8266 is sending data
 {
	 // this is the +IPD rx_buffer - it is quite long.
	 // normally you would not need to copy the whole message in to a variable you can copy up to "HOST" only
	 // or you can just search the data character by character as you read the serial port.
	 
	 bool foundIPD = false;
	 for (int i=0; i<strlen(rx_buffer); i++)
	 {
		 if (  (rx_buffer[i]=='I') && (rx_buffer[i+1]=='P') && (rx_buffer[i+2]=='D')   ) { foundIPD = true;    }
	 }
	 
	 if ( foundIPD  )
	 {
		 loopCount++;
		 // Serial.print( "Have a request.  Loop = ");  Serial.println(loopCount); Serial.println("");
		 
		 // check to see if we have a name - look for name=
		 bool haveName = false;
		 int nameStartPos = 0;
		 for (int i=0; i<strlen(rx_buffer); i++)
		 {
			 if (!haveName) // just get the first occurrence of name
			 {
				 if (  (rx_buffer[i]=='n') && (rx_buffer[i+1]=='a') && (rx_buffer[i+2]=='m') && (rx_buffer[i+3]=='e')  && (rx_buffer[i+4]=='=') )
				 {
					 haveName = true;
					 nameStartPos = i+5;
				 }
			 }
		 }
		 
		 // get the name - copy everything from nameStartPos to the first space character
		 if (haveName)
		 {
			 int tempPos = 0;
			 bool finishedCopying = false;
			 for (int i=nameStartPos; i<strlen(rx_buffer); i++)
			 {
				 if ( (rx_buffer[i]==' ') && !finishedCopying )  { finishedCopying = true;   }
				 if ( !finishedCopying )                     { name[tempPos] = rx_buffer[i];   tempPos++; }
			 }
			 name[tempPos] = 0;
		 }
		
		 // start sending the HTML
		 
		 _delay_ms(1000);
		 
		 at_cipsend(str);
		 /*
		 strcpy(html,"<html><head></head><body>");
		 strcpy(command,"AT+CIPSEND=0,26\r");
		 esp8266_send_cmd(command,100);
		 esp8266_send_cmd(html,100);
		 
		 //at_cipsend("<html><head></head><body>");
		 //at_cipsend("<h1>ESP8266 Webserver</h1>");
		 //at_cipsend("<p>Served by Arduino and ESP8266</p>");
		 
		 strcpy(html,"<h1>ESP8266 Webserver</h1>");
		 strcpy(command,"AT+CIPSEND=0,27\r");
		 esp8266_send_cmd(command,100);
		 esp8266_send_cmd(html,100);
		 
		 strcpy(html,"<p>Served by Arduino and ESP8266</p>");
		 strcpy(command,"AT+CIPSEND=0,36\r");
		 esp8266_send_cmd(command,100);
		 esp8266_send_cmd(html,100);
		 
		 strcpy(html,"<p>Request number ");
		 itoa( loopCount, temp, 10);
		 strcat(html,temp);
		 strcat(html,"</p>");
		 
		 // need the length of html
		 int lenHtml = strlen( html );
		 
		 strcpy(command,"AT+CIPSEND=0,");
		 itoa( lenHtml, temp, 10);
		 strcat(command, temp);
		 strcat(command, "\r");
		 esp8266_send_cmd(command,100);
		 esp8266_send_cmd(html,100);
 
		 if (haveName)
		 {
			 // write name
			 strcpy(html,"<p>Your name is "); strcat(html, name ); strcat(html,"</p>");
			 
			 // need the length of html for the cipsend command
			 lenHtml = strlen( html );
			 strcpy(command,"AT+CIPSEND=0,"); itoa( lenHtml, temp, 10); strcat(command, temp); strcat(command, "\r");
			 esp8266_send_cmd(command,100);
			 esp8266_send_cmd(html,100);
		 }
		 
		 
		 strcpy(html,"<form action=\""); strcat(html, ipAddress); strcat(html, "\" method=\"GET\">"); strcat(command, "\r");
		 
		 lenHtml = strlen( html );
		 itoa( lenHtml, temp, 10);
		 strcpy(command,"AT+CIPSEND=0,");
		 itoa( lenHtml, temp, 10);
		 strcat(command, temp);
		 strcat(command, "\r");
		 
		 esp8266_send_cmd(command,100);
		 esp8266_send_cmd(html,100);
		 
		 strcpy(html,"Name:<br><input type=\"text\" name=\"name\">");
		 strcpy(command,"AT+CIPSEND=0,40\r");
		 esp8266_send_cmd(command,100);
		 esp8266_send_cmd(html,100);
		 
		 strcpy(html,"<input type=\"submit\" value=\"Submit\"></form>");
		 strcpy(command,"AT+CIPSEND=0,43\r");
		 esp8266_send_cmd(command,100);
		 esp8266_send_cmd(html,100);
		 
		 strcpy(html,"</body></html>");
		 strcpy(command,"AT+CIPSEND=0,14\r");
		 esp8266_send_cmd(command,100);
		 esp8266_send_cmd(html,100);
		 */
		 
		 // close the connection
		 esp8266_send_cmd( "AT+CIPCLOSE=0\r", 100);
		 
	 } // if(espSerial.find("+IPD"))
 } //if(espSerial.available())
 
 
 _delay_ms (100);
 
 // drop to here and wait for next request.
}

ISR(USARTD0_RXC_vect) {
	rx_buffer[rx_ptr++] = USARTD0.DATA;
}