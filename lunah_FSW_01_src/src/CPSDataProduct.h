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

#define CPS_EVENT_SIZE	14

typedef struct {
	unsigned char event_id;
	unsigned char n_wPSD_MSB;
	unsigned char n_wPSD_LSB;
	unsigned char n_2cut_MSB;
	unsigned char n_2cut_LSB;
	unsigned char n_noPSD_MSB;
	unsigned char n_noPSD_LSB;
	unsigned char n_bigE_MSB;
	unsigned char n_bigE_LSB;
	unsigned char time_MSB;
	unsigned char time_LSB1;
	unsigned char time_LSB2;
	unsigned char time_LSB3;
	unsigned char modu_temp;
}CPS_EVENT_STRUCT_TYPE;

//Function Prototypes
void cpsSetFirstEventTime( unsigned int time );
void cpsSetRecordedTime( unsigned int m_recorded_time );
float convertToSeconds( unsigned int time );
bool cpsCheckTime( unsigned int time );
CPS_EVENT_STRUCT_TYPE * cpsGetEvent( void );

#endif /* SRC_CPSDATAPRODUCT_H_ */
