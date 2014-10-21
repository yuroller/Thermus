/*
 * DYPTH01
 * library for accessing SURE Electronics DYPTH01 thermal sensor
 *
 * Author: Yuri Valentini, Copyright (c) 2014, all rights reserved
 * Date: 19 Oct 2014
 * Released under the BSD license (http://www.opensource.org/licenses/bsd-license.php)
 */
 
#include "DYPTH01.h"
#include "HWlib.h"

#define FIRST_POLL_DELAY 3 // 30ms
#define OTHER_POLL_DELAY 5 // 50ms
#define POLL_RETRY_COUNT 8
#define DATA_SIZE 4

static unsigned char _data_buf[DATA_SIZE];
static volatile unsigned char _data_cnt = 0;
static int _pin_ss_n = -1;

static const unsigned char CRC8_TABLE[] = {
    0, 49, 98, 83, 196, 245, 166, 151, 185, 136, 219, 234, 125, 76, 31, 46, 67, 114, 33, 16, 135, 182, 229, 212, 250,
    203, 152, 169, 62, 15, 92, 109, 134, 183, 228, 213, 66, 115, 32, 17, 63, 14, 93, 108, 251, 202, 153, 168, 197,
    244, 167, 150, 1, 48, 99, 82, 124, 77, 30, 47, 184, 137, 218, 235, 61, 12, 95, 110, 249, 200, 155, 170, 132, 181,
    230, 215, 64, 113, 34, 19, 126, 79, 28, 45, 186, 139, 216, 233, 199, 246, 165, 148, 3, 50, 97, 80, 187, 138, 217,
    232, 127, 78, 29, 44, 2, 51, 96, 81, 198, 247, 164, 149, 248, 201, 154, 171, 60, 13, 94, 111, 65, 112, 35, 18,
    133, 180, 231, 214, 122, 75, 24, 41, 190, 143, 220, 237, 195, 242, 161, 144, 7, 54, 101, 84, 57, 8, 91, 106,
    253, 204, 159, 174, 128, 177, 226, 211, 68, 117, 38, 23, 252, 205, 158, 175, 56, 9, 90, 107, 69, 116, 39, 22,
    129, 176, 227, 210, 191, 142, 221, 236, 123, 74, 25, 40, 6, 55, 100, 85, 194, 243, 160, 145, 71, 118, 37, 20,
    131, 178, 225, 208, 254, 207, 156, 173, 58, 11, 88, 105, 4, 53, 102, 87, 192, 241, 162, 147, 189, 140, 223,
    238, 121, 72, 27, 42, 193, 240, 163, 146, 5, 52, 103, 86, 120, 73, 26, 43, 188, 141, 222, 239, 130, 179, 224,
    209, 70, 119, 36, 21, 59, 10, 89, 104, 255, 206, 157, 172
};

static unsigned char _calc_crc8(const unsigned char *x, int len)
{
    int i;
    unsigned char crc = 0;
    
    for (i = 0; i < len; ++i) {
        crc = crc ^ x[i];
    }
    return crc;
}

void __attribute__((__interrupt__, no_auto_psv)) _SPI2Interrupt(void)
{
    unsigned char cnt = _data_cnt;
    IFS2bits.SPI2IF = 0;
    if (cnt < DATA_SIZE) {
        _data_buf[cnt] = SPI2BUF;
        cnt++;
        _data_cnt = cnt;
    }
    
    if (DATA_SIZE == cnt) {
        IOPut(_pin_ss_n, ON);
        IEC2bits.SPI2IE = 0; // Disable the interrupt
    }
}

void TH01_InitPort(int pin_sdi, int pin_sdo, int pin_sck, int pin_ss_n)
{
	IOInit(pin_sck, SPICLKIN);
	IOInit(pin_sdo, SPI_OUT);
	IOInit(pin_sdi, SPI_IN);
    IOPut(pin_ss_n, ON);
	IOInit(pin_ss_n, OUT);
    _pin_ss_n = pin_ss_n;

    SPI2BUF = 0;
    IFS2bits.SPI2IF = 0; // Clear the Interrupt flag
    IEC2bits.SPI2IE = 0; // Disable the interrupt
    // SPI2CON1 Register Settings
    SPI2CON1bits.DISSCK = 0; // Internal Serial Clock is enabled
    SPI2CON1bits.DISSDO = 0; // SDOx pin is controlled by the module
    SPI2CON1bits.MODE16 = 0; // Communication is byte-wide (8 bits)
    SPI2CON1bits.SMP = 0; // Input data is sampled at the middle of data output time (must be 0 on slave mode)
    SPI2CON1bits.CKE = 0; // Serial output data changes on transition 
                          // from Idle clock state to active clock state
    SPI2CON1bits.CKP = 0; // Idle state for clock is a low level; active 
                          // state is a high level
    SPI2CON1bits.MSTEN = 0; // Master mode disabled
    SPI2STATbits.SPIROV = 0; // No Receive Overflow has occurred
    SPI2STATbits.SPIEN = 1; // Enable SPI module
                            // Interrupt Controller Settings
    //IFS2bits.SPI2IF = 0; // Clear the Interrupt flag
    //IEC2bits.SPI2IE = 1; // Enable the interrupt
}

int TH01_ReadData(int *t, int *hr)
{
    int i = 0;
    // it takes about 20ms for transmission when SS_NEG is set low and sample is ready
    // it takes 200ms for acquiring the sample
    //unsigned char buf[4];
    IFS2bits.SPI2IF = 0; // Clear the Interrupt flag
    IEC2bits.SPI2IE = 1; // Enable the interrupt
   
    _data_cnt = 0;
    IOPut(_pin_ss_n, OFF);
    vTaskDelay(FIRST_POLL_DELAY); // wait for sample to get ready
    while (DATA_SIZE != _data_cnt && i < POLL_RETRY_COUNT) {
        vTaskDelay(OTHER_POLL_DELAY);
        ++i;
    }

    if (DATA_SIZE != _data_cnt) {
        IOPut(_pin_ss_n, ON);
        IEC2bits.SPI2IE = 0; // Disable the interrupt
        return 1;
    }
    
    if (0 != _calc_crc8(_data_buf, DATA_SIZE)) {
        return 2;
    }
    
    *t = ((int)(_data_buf[0]) << 8) + _data_buf[1] - 400;
    *hr = _data_buf[2];
    return 0;
}
