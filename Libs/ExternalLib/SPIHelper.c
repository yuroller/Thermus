/** \file SPIHelper.c
 *  \brief SPI communication library
 */

/* **************************************************************************
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 *
 *            openSource wireless Platform for sensors and Internet of Things
 * **************************************************************************
 *  FileName:        SPIHelper.c
 *  Module:          FlyPort
 *  Compiler:        Microchip C30 v3.12 or higher
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Claudio Carbone      1.0     10/31/2013        First release  (core team)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code.
 *  OpenPicus software is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/

/**
\defgroup SPI
@{

The SPI library contains all the command to manage the SPI communication module.
*/

#include "SPIHelper.h"

#ifdef SPI_DBG_CFG
char spi_dbg_msg[50];
#endif

/// @cond debug
int _intSPIerr=SPI_NO_ERR;
int _spi_find_divider_index(unsigned int divider);
const int _spi_divider_index[]={2,3,4,5,6,7,8,12,16,20,24,28,32,48,64,80,96,112,128,192,256,320,384,448,512};
const int _spi_divider_primary[]=  {3,3,3,3,3,3,3,
									2,2,2,2,2,
									1,1,1,1,1,1,
									0,0,0,0,0,0,0};
const int _spi_divider_secondary[]={6,5,4,3,2,1,0,
									  5,4,3,2,1,
									6,5,4,3,2,1,
									6,5,4,3,2,1,0};

const int _spi_divider_primary_value[]={64,16,4,1};
const int _spi_divider_secondary_value[]={8,7,6,5,4,3,2,1};
char * const _spi_error_strings[]={"No error\n",
									"Receiver buffer empty\n",
									"Transmitter buffer full\n",
									"Requested bus speed too high\n",
									"Requested a 16bit write, but device configured in 8bit mode\n",
									"Requested a 16bit read, but device configured in 8bit mode\n",
									"Configuration missing either Master or Slave mode\n",
									"Requested a Master operation in Slave mode\n"};
/// @endcond
/**
 * Fuction to enable the SPI communication module
 * \return - 0: the open operation is failed
 * \return - 1: the SPI module is enabled
 */
BOOL SPIOpen()
{
	SPI2STATbits.SPIEN = 1;
	if (SPI2STATbits.SPIEN == 1)
	{
		_intSPIerr=SPI_NO_ERR;
		return 0;
	}
	return 1;
}

/**
 * Function to disable the SPI communication module
 * \return - 0: the close operation is successful
 */
BOOL SPIClose ()
{
	SPI2STATbits.SPIEN = 0;
	_intSPIerr=SPI_NO_ERR;
	return 0;
}

/**
 * Function to read a byte.
 * \param res - pointer in which to store the result
 * \return - 0 the operation is successful
 * \return - 1 the operation is failed. Check internal error for more details
 */
BOOL SPIReadByte (BYTE * res)
{
	int counter=0;
	if(SPI2CON1bits.MSTEN==0)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Master op in Slave mode, quitting\n");
		#endif
		_intSPIerr=NotMasterMode;
		return 1;
	}
	if(SPIWriteByte(0x00)==0)
	{
		while((SPI2STATbits.SPIRBF==0) && (counter!=255))
			counter++;
		if(counter<255)
		{
			#ifdef SPI_DBG
			uartwrite(1,"[SPI_DBG] Rx buf ok, reading\n");
			#endif
			*res = SPI2BUF;
			_intSPIerr=SPI_NO_ERR;
			return 0;
		}
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Rx buf empty, NOT READING!\n");
		#endif
		_intSPIerr = RxBufEmpty;
		return 1;
	}
	
	return 1;
}

/**
 * Function to read a word.
 * \param res - pointer in which to store the result
 * \return - 0 the operation is successful
 * \return - 1 the operation is failed. Check internal error for more details
 */
BOOL SPIReadWord(unsigned int * res)
{
	int counter=0;	
	if(SPI2CON1bits.MSTEN==0)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Master op in Slave mode, quitting\n");
		#endif
		_intSPIerr=NotMasterMode;
		return 1;
	}
	if(SPIWriteWord(0x00)==0)
	{
		while((SPI2STATbits.SPIRBF==0) && (counter!=255))
			counter++;
		if(counter<255)
		{
			#ifdef SPI_DBG
			uartwrite(1,"[SPI_DBG] Rx buf ok, reading\n");
			#endif
			if (SPI2CON1bits.MODE16)
			{
				#ifdef SPI_DBG
				uartwrite(1,"[SPI_DBG] Mode16 ok, reading\n");
				#endif
				*res = SPI2BUF;
				_intSPIerr=SPI_NO_ERR;
				return 0;
			}
			else
			{
				#ifdef SPI_DBG
				uartwrite(1,"[SPI_DBG] Mode16 read, no Mode16 set, NOT READING!\n");
				#endif
				_intSPIerr = Mode16ReadNoMode16;
				return 1;
			}
		}		
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Rx buf empty, NOT READING!\n");
		#endif
		_intSPIerr = RxBufEmpty;
		return 1;
	}
	
	return 1;
}

/**
 * Function to write a byte.
 * \param data - byte data to be sent
 * \return - 0 the operation is successful
 * \return - 1 the operation is failed. Check internal error for more details
 */
BOOL SPIWriteByte(BYTE data)
{
	int counter=0;
	if(SPI2CON1bits.MSTEN==0)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Master op in Slave mode, quitting\n");
		#endif
		_intSPIerr=NotMasterMode;
		return 1;
	}
	while((SPI2STATbits.SPITBF==1) && (counter!=255))
			counter++;
	if(counter<255)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Tx buf not full, writing\n");
		#endif
		SPI2BUF = data;
		_intSPIerr=SPI_NO_ERR;
		return 0;
		
	}
	#ifdef SPI_DBG
	uartwrite(1,"[SPI_DBG] Tx buf full, NOT WRITING!\n");
	#endif
	_intSPIerr = TxBufFul;
	return 1;	
}

/**
 * Function to write a word.
 * \param data - unsigned int data to be sent
 * \return - 0 the operation is successful
 * \return - 1 the operation is failed. Check internal error for more details
 */
BOOL SPIWriteWord(unsigned int data)
{
	int counter=0;
	if(SPI2CON1bits.MSTEN==0)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Master op in Slave mode, quitting\n");
		#endif
		_intSPIerr=NotMasterMode;
		return 1;
	}
	while((SPI2STATbits.SPITBF==1) && (counter!=255))
			counter++;
	if(counter<255)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Tx buf not full, checking 16 mode\n");
		#endif
		if (SPI2CON1bits.MODE16)
		{
			#ifdef SPI_DBG
			uartwrite(1,"[SPI_DBG] 16 mode ok, writing\n");
			#endif
			SPI2BUF = data;
			_intSPIerr=SPI_NO_ERR;
			return 0;
		}
		else
		{
			#ifdef SPI_DBG
			uartwrite(1,"[SPI_DBG] 16 mode not set with 16 mode write request, NOT WRITING!\n");
			#endif
			_intSPIerr = Mode16WriteNoMode16;
			return 1;
		}
	}

	#ifdef SPI_DBG
	uartwrite(1,"[SPI_DBG] Tx buf full, NOT WRITING!\n");
	#endif
	_intSPIerr = TxBufFul;
	return 1;
}

/**
 * Function to perform a contemporary write and read of a BYTE.
 * This is used when the slave device supports full-duplex.
 * In this case while clocking data out, data in is considered valid and saved.
 * \param data - byte data to be sent
 * \param res - pointer to store the received data
 * \return - 0 the operation is successful
 * \return - 1 the operation is failed. Check internal error for more details
 */
BOOL SPIWriteReadByte(BYTE data, BYTE * res)
{
	if(SPI2CON1bits.MSTEN==0)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Master op in Slave mode, quitting\n");
		#endif
		_intSPIerr=NotMasterMode;
		return 1;
	}
	//BOOL outcome;
	if(SPIWriteByte(data)==0)
	{
		//outcome = SPIReadByte(res);
		//return outcome;
		*res = SPI2BUF;
		return 0;
	}
	return 1;
}

/**
 * Function to perform a contemporary write and read of a word.
 * This is used when the slave device supports full-duplex.
 * In this case while clocking data out, data in is considered valid and saved.
 * \param data - unsigned int data to be sent
 * \param res - pointer to store the received data
 * \return - 0 the operation is successful
 * \return - 1 the operation is failed. Check internal error for more details
 */
BOOL SPIWriteReadWord(unsigned int data, unsigned int * res)
{
	if(SPI2CON1bits.MSTEN==0)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Master op in Slave mode, quitting\n");
		#endif
		_intSPIerr=NotMasterMode;
		return 1;
	}
	//BOOL outcome;
	if(SPIWriteWord(data)==0)
	{
		//outcome = SPIReadWord(res);
		//return outcome;
		*res = SPI2BUF;
		return 0;
	}
	return 1;
}

/**
 * Function to set the SPI module.
 * \param obj - SPIContext pointer
 * \param options - SPI options
 * \param pin - SlaveSelect pin, use SPI_OPT_NO_SS if not necessary
 * \param speed - desired connection speed. System will automatically calculate the nearest possible speed
 * \return - 0 the operation is successful
 * \return - 1 the operation is failed. Check internal error for more details
 */
BOOL SPIConfig(SPIContext * obj, int options, int pin, long speed)
{
	double tout=0;
	obj->delay=100;
	obj->SPICON1=0;
	obj->SPICON2=0;
	obj->SPISTAT=0;
	obj->primary=0;
	obj->secondary=0;
	obj->ss_pin=0;
	obj->SPIIE=0;
	obj->SPFIE=0;
	obj->mode16 = FALSE;
	int index=0;
	#ifdef SPI_DBG_CFG
	uartwrite(1,"[SPI_DBG] Initiating configuration\n");
	#endif
	if(speed > _SPI_MAX_SPD)
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] Speed too fast during config, quitting.\n");
		#endif
		_intSPIerr = SpdTooFast;
		return 1;
	}
	unsigned int divider = (unsigned int)ceil(16000000.0/(double)speed);
	#ifdef SPI_DBG_CFG
	sprintf(spi_dbg_msg,"[SPI_DBG] Calculated divider: %d\n",divider);
	UARTWrite(1,spi_dbg_msg);	
	#endif
	index = _spi_find_divider_index(divider);
	obj->primary = _spi_divider_primary[index];
	obj->secondary = _spi_divider_secondary[index];
	obj->ss_pin = pin;
	#ifdef SPI_DBG_CFG
	sprintf(spi_dbg_msg,"[SPI_DBG] primary %d\n[SPI_DBG] secondary %d\n",_spi_divider_primary_value[obj->primary],_spi_divider_secondary_value[obj->secondary]);
	uartwrite(1,spi_dbg_msg);
	#endif	
	tout = ((float)_spi_divider_primary_value[obj->primary] * (float)_spi_divider_secondary_value[obj->secondary])/16; // time (in us) per bit shifted out
	if (options & SPI_OPT_MODE16)
	{
		obj->mode16 = TRUE;
		obj->SPICON1 |= SPI_MSK_MODE16;
		obj->delay = ceil(32*tout/10);
	}
	else
		obj->delay = ceil(16*tout/10);
	if (obj->delay < 1)
		obj->delay = 1;
	#ifdef SPI_DBG_CFG
	sprintf(spi_dbg_msg,"[SPI_DBG] Requested speed %.2f KHz\n[SPI_DBG] Nearest speed %.2f KHz\n",(double)speed/1000,1000/tout);
	uartwrite(1,spi_dbg_msg);
	#endif	
	if((options & SPI_OPT_MODE_0) || (!(options & (SPI_OPT_MODE_0 | SPI_OPT_MODE_1 | SPI_OPT_MODE_2 | SPI_OPT_MODE_3))))
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] Selecting MODE 0\n");
		#endif
		obj->SPICON1 |= SPI_MSK_CPHA; //288 //  bit5=1 master mode + bit8=1 data changes on clock trailing edge
		obj->SPICON1 &= ~SPI_MSK_CPOL;
	}
	
	if(options & SPI_OPT_MODE_1)
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] Selecting MODE 1\n");
		#endif
		obj->SPICON1 &= ~SPI_MSK_CPHA;
 		obj->SPICON1 &= ~SPI_MSK_CPOL; //32// bit5=1 master mode
	}
	
	if(options & SPI_OPT_MODE_2)
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] Selecting MODE 2\n");
		#endif
		obj->SPICON1 |= SPI_MSK_CPHA | SPI_MSK_CPOL; //352 // bit5=1 master mode + bit8=1 data changes on clock trailing edge + bit6=1 clock polarity reversed
	}
		
	if(options & SPI_OPT_MODE_3)
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] Selecting MODE 3\n");
		#endif
		obj->SPICON1 |= SPI_MSK_CPOL;
		obj->SPICON1 &= ~SPI_MSK_CPHA; //96 // bit5=1 master mode + bit6=1 clock polarity
	}
	
	obj->SPICON1 |= obj->primary&0b00000011;
	obj->SPICON1 |= (obj->secondary&0b00011100)<<2;
	if(options & SPI_OPT_MASTER)
		obj->SPICON1 |=  SPI_MSK_MASTER;
	if(options & SPI_OPT_SLAVE)
		obj->SPICON1 &=  ~SPI_MSK_MASTER; //0b1111111111011111;
		
	if(options & SPI_OPT_SLAVE_SELECT)
		obj->SPICON1 |= SPI_MSK_SSEN;
	
	if ((options & (SPI_OPT_SLAVE | SPI_OPT_MASTER)) == 0)
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] No mode (Master/Slave) selected, quitting\n");
		#endif
		_intSPIerr=NoModeSpecified;
		obj->SPICON1=0;
		obj->SPICON2=0;
		obj->SPISTAT=0;
		obj->primary=0;
		obj->secondary=0;
		obj->ss_pin=0;
		return 1;
	}
	/*#if(GroveNest == Board)
	obj->SPICON1 |= 0b1000000;
	#endif*/

	obj->SPISTAT = 0x00;
	obj->SPICON2 = 0x00;
	obj->SPIIE = 0;
	obj->SPFIE = 0;
	obj->IP = 1;
	obj->FIP = 1;
	#ifndef SPI2_FLASH
	if(pin != SPI_OPT_NO_SS)
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] SS pin selected, configuring\n");
		#endif
		obj->ss_pin = pin;
		IOInit(pin,out);
		vTaskDelay(1);
		IOPut(pin,on);
		#ifdef SPI_DBG_CFG
		sprintf(spi_dbg_msg,"[SPI_DBG] SS us of delay %u\n",obj->delay*10);
		UARTWrite(1,spi_dbg_msg);
		#endif
	}
	else
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] SS pin cleared, not using\n");
		#endif
		obj->ss_pin=SPI_OPT_NO_SS;
	}
	#else
	TRISDbits.TRISD6=0;
	#endif
	if(options & SPI_OPT_DI_SAMPLE_END)
	{
		#ifdef SPI_DBG_CFG
		uartwrite(1,"[SPI_DBG] DI sample set to end of DO clock time\n");
		#endif
		obj->SPICON1 |= 0b1000000000;
	}
	_intSPIerr=SPI_NO_ERR;
	return 0;
}

/**
 * Function to save the SPI peripheral status to a SPI Context object for further retrieval.
 * This can be used to manage multiple devices attached to the same SPI.
 * By having multiple contexts it is possible to switch from one to the other including the eventual SlaveSelect pin.
 * This function <b>DOES NOT</b> save the SS pin, this must be done with BOOL SPIConfig(SPIContext * obj, int options, int pin, long speed).
 * \param pointer - SPIContext pointer
 * \return none
 */
void SPIContextSave(SPIContext * obj)
{
	obj->SPICON1 = SPI2CON1;
	obj->SPICON2 = SPI2CON2;
	obj->SPISTAT = SPI2STAT;
	obj->SPIIE	= IEC2bits.SPI2IE;
	obj->SPFIE	= IEC2bits.SPF2IE;
	obj->IP		= IPC8bits.SPI2IP;
	obj->FIP	= IPC8bits.SPF2IP;

}

/**
 * Function to restore the SPI peripheral status to a SPI Context object.
 * This can be used to manage multiple devices attached to the same SPI.
 * By having multiple contexts it is possible to switch from one to the other including the eventual SlaveSelect pin.
 * \param pointer - SPIContext pointer
 * \return none
 */
void SPIContextRestore (SPIContext * obj)
{
	SPI2CON1 = obj->SPICON1;
	SPI2CON2 = obj->SPICON2;
	SPI2STAT = obj->SPISTAT;
	SPI2CON1bits.PPRE = obj->primary;
	SPI2CON1bits.SPRE = obj->secondary;
	IEC2bits.SPI2IE = obj->SPIIE;
	IEC2bits.SPF2IE = obj->SPFIE;
	IPC8bits.SPI2IP = obj->IP;
	IPC8bits.SPF2IP = obj->FIP;
	if(obj->ss_pin != 0)
		IOInit(obj->ss_pin,SPI_SS_OUT);
}

/// @cond debug
int _spi_find_divider_index (unsigned int divider)
{
	int i;
	if(_spi_divider_index[0]==divider)
	{
		#ifdef SPI_DBG_CFG
		UARTWrite(1,"[SPI_DBG] SPI cfg array selected: 0\n");
		#endif
		return 0;
	}
	for (i=1;i<24;i++)
	{
		if (( _spi_divider_index[i-1] < divider ) && ( _spi_divider_index[i] >= divider ))
		{
			#ifdef SPI_DBG_CFG
			sprintf(spi_dbg_msg,"[SPI_DBG] SPI cfg array selected: %d\n",i);
			UARTWrite(1,spi_dbg_msg);
			#endif
			return i;
		}
	}
	
	#ifdef SPI_DBG_CFG
	UARTWrite(1,"[SPI_DBG] SPI cfg array selected: 24\n");
	#endif
	return 24;
}
/// @endcond

/**
 * Function to start a SPI transaction on the given device.
 * This is used to set the SlaveSelect to active before clocking out data.
 * \param pointer - SPIContext pointer
 * \return none
 */
void SPIStart(SPIContext * obj)
{
	#ifndef SPI2_FLASH
	#ifdef SPI_DBG
	uartwrite(1,"[SPI_DBG] SPI start, checking SS\n");
	#endif
	if(obj->ss_pin != SPI_OPT_NO_SS)
	{
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] Pulling SS down\n");
		#endif
		IOPut(obj->ss_pin,off);
	}
	#else
	LATDbits.LATD6=0;
	#endif
}

/**
 * Function to stop a SPI transaction on the given device.
 * This is used to set the SlaveSelect to idle after a transaction occurred thus releasing the bus.
 * \param pointer - SPIContext pointer
 * \return none
 */
void SPIStop(SPIContext * obj)
{
	
	vTaskSuspendAll();
	#ifndef SPI2_FLASH
		#ifdef SPI_DBG
		uartwrite(1,"[SPI_DBG] SPI stop, checking SS\n");
		#endif
		if(obj->ss_pin != SPI_OPT_NO_SS)
		{
			
			#ifdef SPI_DBG
			uartwrite(1,"[SPI_DBG] Pulling SS up\n");
			#endif
			if(SPI2STATbits.SPITBF)
				Delay10us(obj->delay);
			IOPut(obj->ss_pin,on);
		}
	
	#else
		if(SPI2STATbits.SPITBF)
			Delay10us(obj->delay);
		LATDbits.LATD6=1;
	#endif
	xTaskResumeAll();
}

/// @cond debug
void SPIDiscardPreviousData(void)
{
	static unsigned int uiwaste;
	static BYTE bwaste;
	if(SPI2CON1bits.MODE16)
		uiwaste = SPI2BUF;
	else
		bwaste = SPI2BUF;
	SPI2STATbits.SPIROV = 0;
}
/// @endcond

/**
 * Gets an extended description pertinent to the most recent SPI library error.
 * \return pointer to a human readable string describing the error
 */
char * SPIGetErrorString()
{
	return _spi_error_strings[_intSPIerr];
}



