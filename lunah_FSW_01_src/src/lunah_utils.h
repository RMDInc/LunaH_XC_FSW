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
int InitConfig( void );
int SaveConfig( void );
void PutCCSDSHeader(unsigned char * SOH_buff, int length, int packet_type);
void CalculateChecksums(unsigned char * packet_array, int length);
int reportSuccess(XUartPs Uart_PS, int report_filename);
int reportFailure(XUartPs Uart_PS);
int parseCommand(int menusel, char *cmdArr);

// lunah_config structure
// instrument parameters
typedef struct {
	int TriggerThreshold;
	float ECutLoMod1;
	float ECutHiMod1;
	float ECutLoMod2;
	float ECutHiMod2;
	float ECutLoMod3;
	float ECutHiMod3;
	float ECutLoMod4;
	float ECutHiMod4;
	float WideECutLoMod1;
	float WideECutHiMod1;
	float WideECutLoMod2;
	float WideECutHiMod2;
	float WideECutLoMod3;
	float WideECutHiMod3;
	float WideECutLoMod4;
	float WideECutHiMod4;
	float PSDCutLoMod1;
	float PSDCutHiMod1;
	float PSDCutLoMod2;
	float PSDCutHiMod2;
	float PSDCutLoMod3;
	float PSDCutHiMod3;
	float PSDCutLoMod4;
	float PSDCutHiMod4;
	float WidePSDCutLoMod1;
	float WidePSDCutHiMod1;
	float WidePSDCutLoMod2;
	float WidePSDCutHiMod2;
	float WidePSDCutLoMod3;
	float WidePSDCutHiMod3;
	float WidePSDCutLoMod4;
	float WidePSDCutHiMod4;
	int HighVoltageValue[4];
	int IntegrationBaseline;
	int IntegrationShort;
	int IntegrationLong;
	int IntegrationFull;
	float ECalSlope;
	float EcalIntercept;
} CONFIG_STRUCT_TYPE;


#endif /* SRC_LUNAH_UTILS_H_ */
