/*
 * process_data.h
 *
 *  Created on: May 9, 2018
 *      Author: gstoddard
 */

#ifndef SRC_PROCESS_DATA_H_
#define SRC_PROCESS_DATA_H_

#include <string.h>
#include <stdbool.h>
#include "ff.h"
#include "lunah_defines.h"
#include "SetInstrumentParam.h"
#include "CPSDataProduct.h"

typedef struct {
	unsigned char field0;
	unsigned char field1;
	unsigned char field2;
	unsigned char field3;
	unsigned char field4;
	unsigned char field5;
	unsigned char field6;
	unsigned char field7;
}GENERAL_EVENT_TYPE;

//function prototype
int Process2DHData( void );
int ProcessData( unsigned int * data_raw );

#endif /* SRC_PROCESS_DATA_H_ */
