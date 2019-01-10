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
//gives access to last command strings
#include "ReadCommandType.h"
#include "lunah_defines.h"
//let's us get the temperature
#include "LI2C_Interface.h"
//gives access to current filename
#include "DataAcquisition.h"

// prototypes
void InitStartTime( void );
XTime GetLocalTime( void );
int GetNeutronTotal( void );
int GetDigiTemp( void );
int GetAnlgTemp( void );
int GetModuTemp( void );
int CheckForSOH(XIicPs * Iic, XUartPs Uart_PS);
int report_SOH(XIicPs * Iic, XTime local_time, int i_neutron_total, XUartPs Uart_PS, int packet_type);
void PutCCSDSHeader(unsigned char * SOH_buff, int length, int packet_type);
void CalculateChecksums(unsigned char * packet_array, int length);
int reportSuccess(XUartPs Uart_PS, int report_filename);
int reportFailure(XUartPs Uart_PS);

// lunah_config structure
// instrument parameters




#endif /* SRC_LUNAH_UTILS_H_ */
