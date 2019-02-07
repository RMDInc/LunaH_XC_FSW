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
//access to SOH functions
#include "lunah_utils.h"
//access to config buffer
#include "SetInstrumentParam.h"
//access to Polling utilities
#include "ReadCommandType.h"

int GetFileNameSize( void );
char * GetFileName( int file_type );
unsigned int GetDAQRunIDNum( void );
unsigned int GetDAQRunRUNNum( void );
unsigned int GetDAQRunSETNum( void );
int SetFileName( int ID_number, int run_number, int set_number );
int DoesFileExist( void );
int CreateDAQFiles( void );
int WriteRealTime( unsigned long long int real_time );
void ClearBRAMBuffers( void );
int DataAcquisition( XIicPs * Iic, XUartPs Uart_PS, char * RecvBuffer, int time_out );

#endif /* SRC_DATAACQUISITION_H_ */
