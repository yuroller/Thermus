/*
 * Thermus
 * reads temperature e humidity and publish it on xively.com 
 *
 * Author: Yuri Valentini, Copyright (c) 2014, all rights reserved
 * Date: 19 Oct 2014
 * Released under the BSD license (http://www.opensource.org/licenses/bsd-license.php)
 */

#include "taskFlyport.h"
#include "HTTPlib.h"
#include "DYPTH01.h"
#include "xiconfig.h"

/* PINS */
#define PIN_SCK p8
#define PIN_SDO p10 /* unused but needed for stack to work */
#define PIN_SDI p12
#define PIN_SS_N p14

/* XIVELY PARAMETERS */
#define XIVELY_SERVER "api.xively.com"
#define XIVELY_PORT "80"
#define XIVELY_BODY  "{\"version\":\"1.0.0\",\"datastreams\":" \
    "[{\"id\":\"Temperature\",\"current_value\":\"%d.%d\"}," \
    "{\"id\":\"Humidity\",\"current_value\":\"%d\"}]}"

/* XIVELY CALCULATED */
#define XIVELY_HEADER "X-APIKey: " XIVELY_API_KEY "\r\nContent-Type: application/x-www-form-urlencoded\r\n"
#define XIVELY_PATH "/v2/feeds/" XIVELY_FEED_ID

/* DELAYS and TIMEOUTS */
#define POLL_DELAY 20 // 200ms
#define LOOP_DELAY 100 // 1s
#define SOCKET_CONNECT_TIMEOUT 500 // 5s
#define HTTP_TIMEOUT 700 // 7s
#define SENSOR_POLL_INTERVAL 60 // 60s

static char _buf[250];
static char _resp_Body[150];
static char _resp_Header[150];
        
static void _initWifi()
{
	WFConnect(WF_DEFAULT);
	while (WFGetStat() != CONNECTED)
	{
	}
	vTaskDelay(25);
	UARTWrite(1, "Thermus connected...hello world!\r\n");
}

/* timeout in 10ms units */
/* 0 = connected, 1 = timeout */
static int _waitConnection(TCP_SOCKET sock, int timeout)
{
    int retries;
   
    for (retries = timeout / POLL_DELAY; !TCPisConn(sock) && retries > 0; --retries) {
        UARTWrite(1, ".");
        vTaskDelay(POLL_DELAY);
    }
    UARTWrite(1, "\r\n");
    if (!TCPisConn(sock)) {
        UARTWrite(1, "***Unable to connect to server\r\n");
        return 1;
    }
    UARTWrite(1, "Connected to server\r\n");
    return 0;
}

void FlyportTask()
{
    DWORD next_read_tick = TickGetDiv64K();
    
	_initWifi();
    TH01_InitPort(PIN_SDI, PIN_SDO, PIN_SCK, PIN_SS_N);

	while (1)
	{	
        DWORD cur_tick = TickGetDiv64K();
        
        if (next_read_tick > cur_tick) {
            int t = -1;
            int hr = -1;
            int res = -1;
            TCP_SOCKET XivelyClient = INVALID_SOCKET;
        
            next_read_tick = cur_tick + SENSOR_POLL_INTERVAL;
            sprintf(_buf, "Tick = %lu\r\n", cur_tick);
            UARTWrite(1, _buf);

            // discard first read as it is old
            res = TH01_ReadData(&t, &hr);
            if (res != 0) {
                sprintf(_buf, "***Stale TH01_ReadData() Error=%d\r\n", res);
                UARTWrite(1, _buf);
            }
            res = TH01_ReadData(&t, &hr);
            if (res == 0) {
                sprintf(_buf, "Temperature = %d.%d\r\n", t / 10, t % 10);
                UARTWrite(1, _buf);
                sprintf(_buf, "Humidity = %d%%\r\n", hr);
                UARTWrite(1, _buf);
            } else {
                sprintf(_buf, "***Actual TH01_ReadData() Error=%d\r\n", res);
                UARTWrite(1, _buf);
            }
            
            XivelyClient = TCPClientOpen(XIVELY_SERVER, XIVELY_PORT);
            if (0 == _waitConnection(XivelyClient, SOCKET_CONNECT_TIMEOUT)) {
                int resp_code = 0;
                sprintf(_buf, XIVELY_BODY, t / 10, t % 10, hr);
                resp_code = HTTP_Put(XivelyClient, XIVELY_SERVER, XIVELY_PATH, XIVELY_HEADER, _buf,
                    _resp_Header, sizeof(_resp_Header), _resp_Body, sizeof(_resp_Body), HTTP_TIMEOUT);
                if(resp_code == 200) {
					UARTWrite(1, "HTTP request OK\r\n");
				} else {
					UARTWrite(1, "HTTP request ERROR\r\n");
                }
            }
            TCPClientClose(XivelyClient);
     
            vTaskDelay(LOOP_DELAY);
        }
    }
}
