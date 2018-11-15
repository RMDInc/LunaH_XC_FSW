/*
 * main.h
 *
 *  Created on: Apr 24, 2018
 *      Author: gstoddard
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <stdio.h>
// #include <stdlib.h
#include "platform.h"
#include "ps7_init.h"
#include <xil_io.h>
#include <xil_exception.h>
#include "xscugic.h"
#include "xaxidma.h"
#include "xparameters.h"
#include "platform_config.h"
#include "xgpiops.h"
#include "xuartps.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xtime_l.h"
#include "LNumDigits.h"

///SD Card Includes
#include "xparameters.h"	// SDK generated parameters
#include "xsdps.h"			// SD device driver
#include "ff.h"
#include "xil_cache.h"

//Test TEC code
#include "xgpiops.h"

// IiC Interface
#include "LI2C_Interface.h"
#include "xiicps.h"

//File TX
#include "ReadCommandType.h"

//processing
#include "process_data.h"

#include "SetInstrumentParam.h"
#include "lunah_defines.h"
#include "lunah_utils.h"
#include "LogFileControl.h"

#ifdef USE_CCSDS
#define SYNC_MARKER 0x35,0x2e,0xf8,0x53
unsigned char error_buff[] = {SYNC_MARKER, };
unsigned char break_buff[] = {};
unsigned char success_buff[] = {};
#else
unsigned char error_buff[] = "FFFFFF\n";
unsigned char break_buff[] = "FAFAFA\n";
unsigned char success_buff[] = "AAAAAA\n";
#endif
int err_buff_size = sizeof(error_buff);
int break_buff_size = sizeof(break_buff);
int success_buff_size = sizeof(success_buff);


FRESULT ffs_res;
FIL logFile;
FIL data_file;
char filptr_buffer[11] = {};		// Holds 10 numbers and a null terminator
int filptr_clogFile = 0;
char cDirectoryLogFile0[] = "1:/DirectoryFile.txt";	//Directory File to hold all filenames

int iSprintfReturn = 0;
double dTime = 12345.67;
uint numBytesWritten = 0;
uint numBytesRead = 0;

/* I2C Variables */
unsigned char cntrl = 0;
unsigned char addr = 0;
unsigned char i2c_Send_Buffer[2];
unsigned char i2c_Recv_Buffer[2] = {};
int rdac = 0;
int data = 0;

//int *IIC_SLAVE_ADDR;		//pointer to slave
//int IIC_SLAVE_ADDR1 = 0x2F; //Hey Graham, I changed these - mkaffine // 2/21/2018
//int IIC_SLAVE_ADDR1 = 0x20; //HV on the analog board - write to HV pots, RDAC
//int IIC_SLAVE_ADDR2 = 0x4B;	//Temp sensor on digital board
//int IIC_SLAVE_ADDR3 = 0x48;	//Temp sensor on the analog board
//int IIC_SLAVE_ADDR4 = 0x2F;	//VTSET on the analog board - give voltage to TEC regulator
//int IIC_SLAVE_ADDR5 = 0x4A; //Extra Temp Sensor Board, on module near thermistor on TEC

// Global - Interrupt System
u32 global_frame_counter = 0;	// Counts for the interrupt system

// Methods
int InitializeAXIDma( void ); 		// Initialize AXI DMA Transfer
int InitializeInterruptSystem(u16 deviceID);
void InterruptHandler ( void );
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr);
void ClearBuffers();				// Clear Processeed Data Buffers
int ReadDataIn(int numfilesWritten, FIL * filObj);// Take data from DRAM, process it, save it to SD
#ifdef BREAKUP_MAIN
int get_data(XUartPs Uart_PS, char * EVT_filename0, char * CNT_filename0, char * EVT_filename1, char * CNT_filename1, char * RecvBuffer);
#else
int get_data(XUartPs * Uart_PS, char * EVT_filename0, char * CNT_filename0, char * EVT_filename1, char * CNT_filename1, int i_neutron_total, char * RecvBuffer, XTime local_time_start, XTime local_time);						// Print data skipping saving it to SD card
#endif
// moved to lunah_defines.h int report_SOH(XTime local_time, int i_neutron_total, XUartPs Uart_PS);

#endif /* SRC_MAIN_H_ */
