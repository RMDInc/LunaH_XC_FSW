/*
 * CPSDataProduct.h
 *
 *  Created on: Jan 18, 2019
 *      Author: gstoddard
 */

#ifndef SRC_CPSDATAPRODUCT_H_
#define SRC_CPSDATAPRODUCT_H_

#include <stdbool.h>
#include "lunah_utils.h"	//access to module temp
#include "SetInstrumentParam.h"	//access to the neutron cuts

#define CPS_EVENT_SIZE	14

/*
 * This is the CPS event structure and has the follow data fields:
 * 	event ID = 0x55
 * 	n_with_PSD_MSB = neutrons with PSD cuts
 * 	n_with_PSD_LSB
 * 	n_wide_cut_MSB = neutrons with the wide PSD cuts
 * 	n_wide_cut_LSB
 * 	n_with_no_PSD_MSB = neutrons with no PSD cuts, only energy cuts
 * 	n_with_no_PSD_LSB
 * 	high_energy_events_MSB = events with an energy greater than 10 MeV (threshold for energy may change)
 *  high_energy_events_LSB
 *  time_MSB = FPGA time from the beginning of the current 1s interval (extremely important!!!)
 *  time_LSB1
 *  time_LSB2
 *  time_LSB3
 *  modu_temp = the module temperature
 */
typedef struct {
	unsigned char event_id;
	unsigned char n_with_PSD_MSB;
	unsigned char n_with_PSD_LSB;
	unsigned char n_wide_cut_MSB;
	unsigned char n_wide_cut_LSB;
	unsigned char n_with_no_PSD_MSB;
	unsigned char n_with_no_PSD_LSB;
	unsigned char high_energy_events_MSB;
	unsigned char high_energy_events_LSB;
	unsigned char time_MSB;
	unsigned char time_LSB1;
	unsigned char time_LSB2;
	unsigned char time_LSB3;
	unsigned char modu_temp;
}CPS_EVENT_STRUCT_TYPE;

//Function Prototypes
void CPSSetCuts( void );
void CPSInit( void );
void CPSResetCounts( void );
void cpsSetFirstEventTime( unsigned int time );
unsigned int cpsGetFirstEventTime( void );
unsigned int cpsGetCurrentTime( void );
void cpsInitStartTime(void);
float convertToSeconds( unsigned int time );
unsigned int convertToCycles( float time );
bool cpsCheckTime( unsigned int time );
CPS_EVENT_STRUCT_TYPE * cpsGetEvent( void );
bool CPSIsWithinEllipse( double energy, double psd, int module_num, int ellipse_num );
int CPSUpdateTallies(double energy, double psd, int pmt_id);

#endif /* SRC_CPSDATAPRODUCT_H_ */
