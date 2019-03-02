/*
 * DataAcquisition.h
 *
 *  Created on: Dec 4, 2018
 *      Author: IRDLab
 */

#ifndef SRC_DATAACQUISITION_H_
#define SRC_DATAACQUISITION_H_

#include <stdlib.h>
#include <stdio.h>
#include <xil_io.h>
#include "xil_cache.h"
#include "ff.h"
#include <string.h>
#include "xscugic.h"
#include "lunah_defines.h"
#include "sleep.h"
//TODO: Get the nanosleep function for Meg
#include "process_data.h"
#include "lunah_utils.h"
#include "SetInstrumentParam.h"
#include "ReadCommandType.h"

//Interrupt Variables
extern XScuGic InterruptController;		// Interrupt controller

int GetFileNameSize( void );
char *GetFileName( int file_type );
unsigned int GetDAQRunIDNum( void );
unsigned int GetDAQRunRUNNum( void );
unsigned int GetDAQRunSETNum( void );
int SetFileName( int ID_number, int run_number, int set_number );
int DoesFileExist( void );
int CreateDAQFiles( void );
FIL *GetEVTFilePointer( void );
FIL *GetCPSFilePointer( void );
FIL *Get2DHFilePointer( void );
int WriteRealTime( unsigned long long int real_time );
void ClearBRAMBuffers( void );
int DataAcquisition( XIicPs * Iic, XUartPs Uart_PS, char * RecvBuffer, int time_out );

#endif /* SRC_DATAACQUISITION_H_ */
