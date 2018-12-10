/*
 * DataAcquisition.h
 *
 *  Created on: Dec 4, 2018
 *      Author: IRDLab
 */

#ifndef SRC_DATAACQUISITION_H_
#define SRC_DATAACQUISITION_H_

#include <stdio.h>
#include "ff.h"
#include "lunah_defines.h"

//FILE SCOPE STRUCTS
//the size of this struct is 20 bytes
struct event_CPS_dp
{
	unsigned char event_id;
	unsigned char real_time_MSB;
	unsigned char real_time_LSB7;
	unsigned char real_time_LSB6;
	unsigned char real_time_LSB5;
	unsigned char real_time_LSB4;
	unsigned char real_time_LSB3;
	unsigned char real_time_LSB2;
	unsigned char real_time_LSB1;
	unsigned char FPGA_time_MSB;
	unsigned char FPGA_time_LSB3;
	unsigned char FPGA_time_LSB2;
	unsigned char FPGA_time_LSB1;
	char module_temp;
};

char * GetFileName( int file_type );
int SetFileName( int ID_number, int run_number );
int DoesFileExist( void );
int WriteHeaderFile( unsigned long long int real_time, float modu_temp );

#endif /* SRC_DATAACQUISITION_H_ */
