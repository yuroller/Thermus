#include "taskFlyport.h"
#include "SPIHelper.h"
#include "HTTPlib.h"

#include <time.h>

#define DELAY_RETRY_INIT_SNTP 50 // 10ms units
#define EPOCH_SANE_VALUE 1413299584 // 14 Oct 2014 17:13

#define PIN_SCLK p8
//#define PIN_MOSI p10
#define PIN_MISO p12
#define PIN_CS p14

static SPIContext g_spiCtx;

static void _initWifi()
{
	WFConnect(WF_DEFAULT);
	while (WFGetStat() != CONNECTED)
	{
	}
	vTaskDelay(25);
	UARTWrite(1, "Thermus connected...hello world!\r\n");
}

static void _initRtc()
{
	char s[50];
	struct tm *ts;
	time_t now;
	DWORD epoch = SNTPGetUTCSeconds();

	while (epoch < EPOCH_SANE_VALUE)
	{
		UARTWrite(1, "Retrying init from SNTP!\r\n");
		vTaskDelay(DELAY_RETRY_INIT_SNTP);
	}
	
	now = (time_t)epoch;
	ts = gmtime(&now);
	RTCCSet(ts);
	sprintf(s, "Setting clock to: %s\r\n", asctime(ts));
	UARTWrite(1, s);
}
 
static void _initSpi()
{
	IOInit(PIN_SCLK, SPICLKOUT);
	//IOInit(PIN_MOSI, SPI_OUT);
	IOInit(PIN_MISO, SPI_IN);
	IOPut(PIN_CS, ON);
	IOInit(PIN_CS, OUT);
   	SPIConfig(&g_spiCtx, SPI_OPT_SLAVE | SPI_OPT_MODE_0, PIN_CS, 250000);
	SPIContextRestore(&g_spiCtx);
	SPIOpen();
}
  
static void _initAlarm()
{
	struct tm ts;
	RTCCGet(&ts);
	ts.tm_min = 0;
	ts.tm_sec = 0;
	RTCCAlarmConf(&ts, REPEAT_INFINITE, EVERY_MIN, NO_ALRM_EVENT);
}

void FlyportTask()
{

	// Flyport connects to default network

	_initWifi();
	_initRtc();
    _initSpi();
	_initAlarm();

	while(1)
	{	
		
		// INSERT HERE YOUR CODE
	}
}
