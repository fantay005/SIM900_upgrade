#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "m35ftp.h"
#include "m35at.h"
#include "inside_flash.h"

static char buffer[1024];

bool m35FtpConnect(const char *remoteFile, const char *host, int port, int timeoutMs)
{
	char buf[80];
	sprintf(buf, "AT+FTPSERV=\"%s\"\n", host);
	if (!m35AtChat(buf, "OK", buf, timeoutMs)) return false;
	
	sprintf(buf, "AT+FTPPORT=%d\n", port);
	if (!m35AtChat(buf, "OK", buf, timeoutMs)) return false;
	
	sprintf(buf, "AT+FTPUN=\"ftpdown\"\n");
	if (!m35AtChat(buf, "OK", buf, timeoutMs)) return false;
	
	sprintf(buf, "AT+FTPPW=\"fitbright\"\n");
	if (!m35AtChat(buf, "OK", buf, timeoutMs)) return false;
	
	sprintf(buf, "AT+FTPGETNAME=\"%s\"\n", remoteFile);
	if (!m35AtChat(buf, "OK", buf, timeoutMs)) return false;
	
	sprintf(buf, "AT+FTPGETPATH=\"/\"\n");
	if (!m35AtChat(buf, "OK", buf, timeoutMs)) return false;
	return true;
	}

extern void delayMs(unsigned int ms);

bool m35FtpDownload(void)
{
	char buf[128], tmp[5];
	int len = 0, i = 0;
	
	while(1){
		sprintf(buf, "AT+FTPSTATE\n");
		if (!m35AtChat(buf, "+FTPSTATE: ", buf, 1000000)) return false;
		for (len = 11; len < strlen(buf); len++) {
			if (isdigit(buf[len]) || buf[len] == '-') break;
		}
		if (len >= strlen(buf)) return false;
		if (0 != atoi(&buf[len])) {
			delayMs(1000);
			continue;
		}
		
		break;
	}

	sprintf(buf, "AT+FTPGET=1\n");
	if (!m35AtChat(buf, "OK", buf, 20000)) return false;
	
	if (!m35AtChat(NULL, "+FTPGET: 1,", buf, 20000)) return false;
	for (len = 10; len < strlen(buf); len++) {
		if (isdigit(buf[len])) break;
	}
	if (len >= strlen(buf)) return false;
	if (1 != atoi(&buf[len])) return false;

	delayMs(2000);
	
	while(1){			
		sprintf(buf, "AT+FTPGET=2,1024\n");
		if (!m35AtChat(buf, "+FTPGET: 2,", buf, 10000)) return false;
		for (len = 10; len < strlen(buf); len++) {
			if (isdigit(buf[len]) || buf[len] == '-') break;
		}
		if (len >= strlen(buf)) return false;
		
		sscanf(buf, "%*[^,]%*c%s", tmp);
		len = atoi((const char *)tmp);
		m35ReceiveData(buffer, len, 500);
		STMFLASH_Write(Download_Store_Addr + i * 1024, (uint16_t *)buffer, len / 2);
		i++;
		if (len < 1024)
			break;
		delayMs(300);		
	}

	return true;
}
