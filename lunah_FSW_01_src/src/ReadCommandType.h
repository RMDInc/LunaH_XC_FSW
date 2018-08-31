/*
 * ReadCommandType.h
 *
 *  Created on: Apr 12, 2018
 *      Author: gstoddard
 */

#ifndef SRC_READCOMMANDTYPE_H_
#define SRC_READCOMMANDTYPE_H_

#include <stdio.h>		//needed for unsigned types
#include "xuartps.h"	//needed for uart functions

//global variables
//char commandBuffer[20] = "";
//char commandBuffer2[50] = "";



int ReadCommandType(char * RecvBuffer, XUartPs *Uart_PS);
int PollUart(char * RecvBuffer, XUartPs *Uart_PS);

// Command definitions (can optionally be done with enum - used defines for now to assure compatibility)
#define DAQ__CMD 0
#define WF_CMD 1
#define TMP_CMD 2
#define GETSTAT_CMD 3
#define DISABLE_ACT_CMD 4
#define DISABLE_TEC_CMD 5
#define ENABLE_TEC_CMD 6
#define TX_CMD 7
#define DEL_CMD 8
#define LS_CMD 9
#define TRG_CMD 10
#define NGATES_CMD 11
#define HV_CMD 12
#define INT_CMD 13
#define ECAL_CMD 14
#define BREAK_CMD 15
#define START_CMD 16
#define END_CMD 17
#define END_TMP_CMD 18
#define READ_TMP_CMD 19

#define INPUT_OVERFLOW 100
#define NO_CMD_INPUT 999



#endif /* SRC_READCOMMANDTYPE_H_ */
