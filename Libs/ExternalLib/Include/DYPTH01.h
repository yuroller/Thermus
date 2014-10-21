#ifndef DYPTH01_H_
#define DYPTH01_H_

/*
 * DYPTH01
 * library for accessing SURE Electronics DYPTH01 thermal sensor
 *
 * Author: Yuri Valentini, Copyright (c) 2014, all rights reserved
 * Date: 19 Oct 2014
 * Released under the BSD license (http://www.opensource.org/licenses/bsd-license.php)
 */

/*! Initializes SPI port for communication */
/*!
  \param[in] pin_sdi SPI input pin
  \param[in] pin_sdo SPI output pin (unused but needed for hw SPI module)
  \param[in] pin_sck SPI input clock
  \param[in] pin_ss_n chip select
*/
void TH01_InitPort(int pin_sdi, int pin_sdo, int pin_sck, int pin_ss_n);

/*! Read last acquired sample */
/*!
  \param[out] t temperature in 0.1C units
  \param[out] hr relative humidity in percent
  \return 0=ok, 1=timeout_error, 2=crc_error
*/
int TH01_ReadData(int *t, int *hr);

#endif // !DYPTH01_H_
