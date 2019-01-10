/*
 * DataAcquisition.h
 *
 *  Created on: Dec 4, 2018
 *      Author: IRDLab
 */

#ifndef SRC_DATAACQUISITION_H_
#define SRC_DATAACQUISITION_H_

//standard libraries
#include <stdlib.h>
//basic C I/O
#include <stdio.h>
//access to the Xilinx I/O commands to talk w/the FPGA
#include <xil_io.h>
//access cache handling functions
#include "xil_cache.h"
//include the SD card library
#include "ff.h"
//used to get strlen
#include <string.h>
#include "lunah_defines.h"
//access to usleep for the smallest sleeps we can manage
#include "sleep.h"
//TODO: Get the nanosleep function for Meg

//access to processing functions
#include "process_data.h"

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

int GetFileNameSize( void );
char * GetFileName( int file_type );
unsigned int GetDAQRunIDNum( void );
unsigned int GetDAQRunRUNNum( void );
unsigned int GetDAQRunSETNum( void );
int SetFileName( int ID_number, int run_number, int set_number );
int DoesFileExist( void );
int CreateDAQFiles( void );
int WriteDataFileHeader( unsigned long long int real_time, float modu_temp );
void ClearBRAMBuffers( void );
int DataAcquisition( void );

#endif /* SRC_DATAACQUISITION_H_ */
