/*
 * main.h
 *
 *  Created on: Apr 24, 2018
 *      Author: gstoddard
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <stdio.h>
#include "platform.h"
#include "ps7_init.h"
#include <xil_io.h>
#include <xil_exception.h>
#include "xscugic.h"
#include "xaxidma.h"
#include "xparameters.h"	// SDK generated parameters
#include "platform_config.h"
#include "xgpiops.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xtime_l.h"
//SD CARD INCLUDES
#include "xsdps.h"			// SD device driver
#include "ff.h"
#include "xil_cache.h"
//IIC INTERFACE
#include "LI2C_Interface.h"
#include "xiicps.h"
//USER INPUT/POLLING
#include "ReadCommandType.h"
//PROCESSING
#include "process_data.h"
//GJS CODE LIBRARIES
#include "SetInstrumentParam.h"
#include "lunah_defines.h"
#include "lunah_utils.h"
#include "LogFileControl.h"
#include "DataAcquisition.h"
#include "LNumDigits.h"

//Global Interrupt Control Variables
//These need to be global for interrupts to be handled appropriately within the system
static XScuGic_Config *GicConfig; 	// Configuration parameters of the controller
XScuGic InterruptController;		// Interrupt controller

// Methods
int InitializeAXIDma( void ); 		// Initialize AXI DMA Transfer
int InitializeInterruptSystem(u16 deviceID);
void InterruptHandler ( void );
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr);

#endif /* SRC_MAIN_H_ */
