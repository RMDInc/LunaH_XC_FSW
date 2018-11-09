/*
 * lunah_utils.h
 *
 *  Created on: Jun 22, 2018
 *      Author: IRDLAB
 */

#ifndef SRC_LUNAH_UTILS_H_
#define SRC_LUNAH_UTILS_H_

#include <xtime_l.h>
#include <xuartps.h>
#include "ff.h"

// prototypes
void InitStartTime(void);
XTime GetLocalTime(void);
int GetNeutronTotal(void);
int CheckForSOH(XUartPs Uart_PS);
int report_SOH(XTime local_time, int i_neutron_total, XUartPs Uart_PS, int packet_type);
int InitConfig(void);
int SaveConfig(void);
void PutCCSDSHeader(unsigned char * SOH_buff, int length);
void CalculateChecksums(unsigned char * packet_array, int length);


// lunah_config structure
// instrument parameters
typedef struct {
	int TriggerThreshold;
	float EnergyCut[2];
	float PsdCut[2];
	float WideEnergyCut[2];
	float WidePsdCut[2];
	int HighVoltageValue[4];
	int IntegrationBaseline;
	int IntegrationShort;
	int IntegrationLong;
	int IntegrationFull;
	float ECalSlope;
	float EcalIntercept;
} CONFIG_STRUCT_TYPE;


// lunah config defaults
#define DFLT_TRIGG_THRES
#define DFLT_ENERGY_CUT_1
#define DFLT_ENERGY_CUT_2


#endif /* SRC_LUNAH_UTILS_H_ */
