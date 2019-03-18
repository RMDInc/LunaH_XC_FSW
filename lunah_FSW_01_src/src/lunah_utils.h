/*
 * lunah_utils.h
 *
 *  Created on: Jun 22, 2018
 *      Author: IRDLAB
 */

#ifndef SRC_LUNAH_UTILS_H_
#define SRC_LUNAH_UTILS_H_

#include <stdlib.h>
#include <xtime_l.h>
#include <xuartps.h>
#include "ff.h"
#include "ReadCommandType.h"	//gives access to last command strings
#include "lunah_defines.h"		//let's us get the temperature
#include "LI2C_Interface.h"
#include "DataAcquisition.h"	//gives access to current filename

#define TAB_CHAR_CODE			9
#define NEWLINE_CHAR_CODE		10
#define SOH_PACKET_LENGTH		29
#define TEMP_PACKET_LENGTH		19

// prototypes
void InitStartTime( void );
XTime GetLocalTime( void );
XTime GetTempTime(void);
int GetNeutronTotal( void );
int PutNeutronTotal(int total);
int IncNeutronTotal(int increment);
int GetDigiTemp( void );
int GetAnlgTemp( void );
int GetModuTemp( void );
int CheckForSOH(XIicPs * Iic, XUartPs Uart_PS);
int report_SOH(XIicPs * Iic, XTime local_time, int i_neutron_total, XUartPs Uart_PS, int packet_type);
void PutCCSDSHeader(unsigned char * SOH_buff, int length, int packet_type);
int reportSuccess(XUartPs Uart_PS, int report_filename);
int reportFailure(XUartPs Uart_PS);
void CalculateChecksums(unsigned char * packet_array);
int TransferSDFile( XUartPs Uart_PS, int file_to_access );

#endif /* SRC_LUNAH_UTILS_H_ */
