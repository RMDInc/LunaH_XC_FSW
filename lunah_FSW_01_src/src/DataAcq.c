/*
 * DataAcq.c
 *
 *  Created on: Jul 9, 2018
 *      Author: ellis
 */

#include <stdio.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "xuartps.h"
#include "lunah_defines.h"
#include "ReadCommandType.h"

enum LoopStateTypes LoopState = MainLoopState;
unsigned int RealTime;

//extern XUartPs Uart_PS;

int DataAcqInit(int Command, int i_orbit_number)
{
  int bytes_sent;
  int RetVal;
  char report_buff[100];
  char c_CNT_Filename0[FILENAME_SIZE] = "";
  char c_EVT_Filename0[FILENAME_SIZE] = "";
  char c_CNT_Filename1[FILENAME_SIZE] = "";
  char c_EVT_Filename1[FILENAME_SIZE] = "";

  static int i_daq_run_number;


	i_daq_run_number++;
	// Energize Detectors, Front End and ADC


	if(Command == WF_CMD)
	{
		// set bypass

	}

	// Create data file names

		snprintf(c_EVT_Filename0, 50, "0:/%04d_%04d_evt.bin",i_orbit_number, i_daq_run_number);	//assemble the filename
		snprintf(c_CNT_Filename0, 50, "0:/%04d_%04d_cnt.bin",i_orbit_number, i_daq_run_number);
		snprintf(c_EVT_Filename1, 50, "1:/%04d_%04d_evt.bin",i_orbit_number, i_daq_run_number);
		snprintf(c_CNT_Filename1, 50, "1:/%04d_%04d_cnt.bin",i_orbit_number, i_daq_run_number);
		RetVal = snprintf(report_buff, 100, "%s_%s\n", c_EVT_Filename0, c_CNT_Filename0);	//create the string to tell CDH
		//bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, RetVal);
	// Open data files



	// set state to wait for start cmd

	LoopState = WaitStartState;

	return 0;
}



int StartDataAcqLoop(int i_orbit_number)
{
  int Command;
  int RetVal;

  char RecvBuffer[100];



	//Command = ReadCommandType(RecvBuffer, &Uart_PS);
	switch(Command)
	{
		case START_CMD:
		    RetVal = sscanf(RecvBuffer + strlen("START_"), " %d", &RealTime);
		    if(RetVal)
		    {
		       // handle error and return
		    }
		    Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 1);	//enable capture module
			//the above triggers a false event which we use to sync the S/C time
			// with the local FPGA time
			Xil_Out32(XPAR_AXI_GPIO_6_BASEADDR, 1);		//enable ADC
			Xil_Out32 (XPAR_AXI_GPIO_7_BASEADDR, 1);	//enable 5V to analog board
	      	LoopState = CollectDataState;

		  	break;
		case BREAK_CMD:
	        LoopState = MainLoopState;
		    break;
		case READ_TMP_CMD:

     	  	break;

		default:
			break;

 	}

	return 0;
}

/*
 * EndDataAcqLoop
 *     Collect data, process data and save data to SD card.
 */
int CollectDataLoop(int Command)
{

    switch(Command)
    {
        case READ_TMP_CMD:

           break;
    	case END_CMD:

    	   break;

    	case BREAK_CMD:

    	   break;

    	default:
    		break;
    }

    return 0;
}


int DataAcqLoop(int Command, int i_orbit_number)
{
  char RecvBuffer[100];

    DataAcqInit(Command, i_orbit_number);

    StartDataAcqLoop(i_orbit_number);

    CollectDataLoop(i_orbit_number);

    return 0;
}
