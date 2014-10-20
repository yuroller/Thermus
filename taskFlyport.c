#include "taskFlyport.h"
#include "HTTPlib.h"
#include "DYPTH01.h"

#define PIN_SCK p8
#define PIN_SDO p10
#define PIN_SDI p12
#define PIN_SS_N p14

/* XIVELY PARAMETERS */
//const char XivelyServer[] = "api.xively.com";
//const char XivelyPort[] = "80";
//const char XivelyAPIKey[250];
//char XivelyFeedID[250];
//TCP_SOCKET XivelyClient = INVALID_SOCKET;

static void _initWifi()
{
	WFConnect(WF_DEFAULT);
	while (WFGetStat() != CONNECTED)
	{
	}
	vTaskDelay(25);
	UARTWrite(1, "Thermus connected...hello world!\r\n");
}
  
void FlyportTask()
{

	// Flyport connects to default network

	_initWifi();
    TH01_InitPort(PIN_SDI, PIN_SDO, PIN_SCK, PIN_SS_N);

	while(1)
	{	
		int t = -1;
        int hr = -1;
        char buf[50];
        int res = TH01_ReadData(&t, &hr);
        if (res == 0) {
            sprintf(buf, "Temperature = %d.%d", t / 10, t % 10);
            UARTWrite(1, buf);
            sprintf(buf, "Humidity = %d%%", hr);
            UARTWrite(1, buf);
        } else {
            sprintf(buf, "***TH01_ReadData() Error=%d", res);
            UARTWrite(1, buf);
        }
		vTaskDelay(100);
	}
}
