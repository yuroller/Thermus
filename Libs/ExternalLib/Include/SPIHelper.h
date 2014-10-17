/** \file SPIHelper.h
 *  \brief SPI communication library
 */

/**
\mainpage Main page

 Library to manage the SPI communication module.
*/

#ifndef _FLYPORT_SPI_
#define _FLYPORT_SPI_

#include "taskFlyport.h"
#include "HWlib.h"
#include "math.h"

//#define SPI_DBG
//#define SPI_DBG_CFG
//#define SPI2_FLASH

/**
 * SPI Options
 * Either SPI_OPT_MASTER or SPI_OPT_SLAVE <b>must</b> always be present. Function will give error otherwise.
 * Options can be chained with an or operator, ex: <i>opt1 | opt2 | opt3</i>
<B>Possible options available for the SPI configuration: </B>
 <UL>
	<LI><B>SPI_OPT_MASTER:</B> Enable master mode, ex: SPIConfig(obj,SPI_OPT_MASTER,pin,speed)</LI><BR>
	<LI><B>SPI_OPT_SLAVE:</B> Enable slave mode (word), ex: SPIConfig(obj,SPI_OPT_SLAVE,pin,speed)</LI><BR>
	<LI><B>SPI_OPT_MODE_0:</B> Use mode 0 (default) to drive SCK and DO, ex: SPIConfig(obj,SPI_OPT_MODE_0 | SPI_OPT_MASTER,pin,speed)</LI><BR>
	<LI><B>SPI_OPT_MODE_1:</B> Use mode 1 to drive SCK and DO, ex: SPIConfig(obj,SPI_OPT_MODE_1 | SPI_OPT_MASTER,pin,speed)</LI><BR>
	<LI><B>SPI_OPT_MODE_2:</B> Use mode 2 to drive SCK and DO, ex: SPIConfig(obj,SPI_OPT_MODE_2 | SPI_OPT_MASTER,pin,speed)</LI><BR>
	<LI><B>SPI_OPT_MODE_3:</B> Use mode 3 to drive SCK and DO, ex: SPIConfig(obj,SPI_OPT_MODE_3 | SPI_OPT_MASTER,pin,speed)</LI><BR>
	<LI><B>SPI_OPT_MODE16:</B> Activate 16 mode (word), ex: SPIConfig(obj,SPI_OPT_MODE16 | SPI_OPT_MASTER,pin,speed)</LI><BR>
	<LI><B>SPI_OPT_NO_SS:</B> Disable use of SlaveSelect for the current device, ex: SPIConfig(obj,SPI_OPT_MASTER,SPI_OPT_NO_SS,speed)</LI><BR>
	<LI><B>SPI_OPT_DI_SAMPLE_END:</B> Sample DataInput line at end of clock period instead of at middle, ex: SPIConfig(obj,SPI_OPT_DI_SAMPLE_END | SPI_OPT_MASTER,pin,speed)</LI><BR>
	<LI><B>SPI_OPT_SLAVE_SELECT:</B> Only valid in Slave mode, enable the chip select pin (SPI is idle if SS is idle), ex: SPIConfig(obj,SPI_OPT_SLAVE_SELECT | SPI_OPT_SLAVE,pin,speed)</LI><BR>
	 
 </ul>
 */
#define 	SPI_OPT_MODE16 		0b1
#define		SPI_OPT_MODE_0		0b10
#define		SPI_OPT_MODE_1		0b100
#define		SPI_OPT_MODE_2		0b1000
#define		SPI_OPT_MODE_3		0b10000
#define		SPI_OPT_MASTER		0b100000
#define		SPI_OPT_SLAVE		0b1000000
#define		SPI_OPT_SLAVE_SELECT	0b10000000
#define		SPI_OPT_DI_SAMPLE_END	0b100000000
#define		SPI_OPT_NO_SS		0xff

#define		SPI_MSK_MASTER		0x20
#define		SPI_MSK_CPOL		0x40
#define		SPI_MSK_SSEN		0x80
#define		SPI_MSK_CPHA		0x100
#define		SPI_MSK_SMP		0x200
#define		SPI_MSK_MODE16		0x400
#define		_SPI_MAX_SPD		8000000

#define		SPI_NO_ERR		0
#define 	RxBufEmpty 		1
#define 	TxBufFul		2
#define		SpdTooFast		3
#define		Mode16WriteNoMode16 	4
#define		Mode16ReadNoMode16	5
#define		NoModeSpecified		6
#define		NotMasterMode		7

/*****************************************************************************
        SPI struct declaration
*****************************************************************************/

struct  _SPI_TYPE
{
	int SPICON1;
	int SPICON2;
	int SPISTAT;
	BOOL SPIIE;
	BOOL SPFIE;
	BYTE IP;
	BYTE FIP;
	BYTE ss_pin;
	BYTE primary;
	BYTE secondary;
	BOOL mode16;
	unsigned int delay;
	
};

typedef struct _SPI_TYPE SPIContext;

/*****************************************************************************
        SPI function declarations
*****************************************************************************/

BOOL SPIOpen();
BOOL SPIReadByte(BYTE * res);
BOOL SPIReadWord(unsigned int * res);
BOOL SPIWriteByte(BYTE data);
BOOL SPIWriteWord(unsigned int data);
BOOL SPIWriteReadByte(BYTE data, BYTE * res);
BOOL SPIWriteReadWord(unsigned int data, unsigned int * res);
int SPIStatus();
BOOL SPIClose();
BOOL SPIConfig(SPIContext * obj, int options, int pin, long speed);
void SPIContextSave(SPIContext * obj);
void SPIContextRestore (SPIContext * obj);
void SPIStart(SPIContext * obj);
void SPIStop(SPIContext * obj);
void SPIDiscardPreviousData(void);
char * SPIGetErrorString();
#endif
