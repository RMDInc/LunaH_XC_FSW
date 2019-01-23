/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * Mini-NS Flight Software, Version 4.1
 * Graham Stoddard
 *
 * 1-9-2019
 *
 */

#include "main.h"

int main()
{
    // Initialize System
	ps7_post_config();
	Xil_DCacheDisable();	// Disable the L1/L2 data caches
	InitializeAXIDma();		// Initialize the AXI DMA Transfer Interface
	InitializeInterruptSystem(XPAR_PS7_SCUGIC_0_DEVICE_ID);

	// *********** Setup the Hardware Reset GPIO ****************//
	// GPIO/TEC Test Variables
	XGpioPs Gpio;
	int gpio_status = 0;
	XGpioPs_Config *GPIOConfigPtr;

	GPIOConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	gpio_status = XGpioPs_CfgInitialize(&Gpio, GPIOConfigPtr, GPIOConfigPtr->BaseAddr);
	if(gpio_status != XST_SUCCESS)
		xil_printf("GPIO PS init failed\r\n");

	XGpioPs_SetDirectionPin(&Gpio, TEC_PIN, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, TEC_PIN, 1);
	XGpioPs_WritePin(&Gpio, TEC_PIN, 0);	//disable TEC startup

	XGpioPs_SetDirectionPin(&Gpio, SW_BREAK_GPIO, 1);
	//******************Setup and Initialize IIC*********************//
	int iic_status = 0;
	//Make the IIC Instance come from here and we pass it in to the functions
	XIicPs Iic;

	iic_status = IicPsInit(&Iic, IIC_DEVICE_ID_0);
	if(iic_status != XST_SUCCESS)
	{
		//handle the issue
		xil_printf("fix the Iic device 0\r\n");
	}
	iic_status = IicPsInit(&Iic, IIC_DEVICE_ID_1);
	if(iic_status != XST_SUCCESS)
	{
		//handle the issue
		xil_printf("fix the Iic device 1\r\n");
	}

	//*******************Receive and Process Packets **********************//
	Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, 0);	//baseline integration time	//subtract 38 from each int
	Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, 35);	//short
	Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, 131);	//long
	Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, 1513);	//full
	Xil_Out32 (XPAR_AXI_GPIO_4_BASEADDR, 0);	//TEC stuff, 0 turns things off
	Xil_Out32 (XPAR_AXI_GPIO_5_BASEADDR, 0);	//TEC stuff
	Xil_Out32 (XPAR_AXI_GPIO_6_BASEADDR, 0);	//enable the system, allows data
	Xil_Out32 (XPAR_AXI_GPIO_7_BASEADDR, 0);	//enable 5V to sensor head
	Xil_Out32 (XPAR_AXI_GPIO_10_BASEADDR, 8500);	//threshold, max of 2^14 (16384)
	Xil_Out32 (XPAR_AXI_GPIO_16_BASEADDR, 16384);	//master-slave frame size
	Xil_Out32 (XPAR_AXI_GPIO_17_BASEADDR, 1);	//master-slave enable
	Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 0);	//capture module enable

	//*******************Setup the UART **********************//
	int Status = 0;
	int LoopCount = 0;
	XUartPs Uart_PS;	// instance of UART

	XUartPs_Config *Config = XUartPs_LookupConfig(UART_DEVICEID);
	if (Config == NULL) { return 1;}
	Status = XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);
	if (Status != 0){ xil_printf("XUartPS did not CfgInit properly.\n");	}

	/* Conduct a Selftest for the UART */
	Status = XUartPs_SelfTest(&Uart_PS);
	if (Status != 0) { xil_printf("XUartPS failed self test.\n"); }			//handle error checks here better

	/* Set to normal mode. */
	XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);
	while (XUartPs_IsSending(&Uart_PS)) {
		LoopCount++;
	}
	// *********** Mount SD Card ****************//
	/* FAT File System Variables */
	FATFS fatfs[2];
	int sd_status = 0;

	sd_status = MountSDCards( fatfs );
	if(sd_status == CMD_SUCCESS)	//correct mounting
	{
		sd_status = InitLogFile0();	//create log file on SD0
		if(sd_status == CMD_FAILURE)
		{
			//handle a bad log file?
			xil_printf("SD0 failed to init\r\n");
		}
		sd_status = InitLogFile1();	//create log file on SD1
		if(sd_status == CMD_FAILURE)
		{
			//handle a bad log file?
			xil_printf("SD1 failed to init\r\n");
		}
	}
	else
	{
		xil_printf("SD0/1 failed to mount\r\n");
		//need to handle the SD card not reading
		//do we try each one separately then set a flag?
		sd_status = MountSD0(fatfs);
		if(sd_status == CMD_SUCCESS)
		{
			//SD0 is not the problem
		}
		else
		{
			//SD0 is the problem
			//set a flag to indicate to only use SD1?
			xil_printf("SD0 failed to mount\r\n");
		}
		sd_status = MountSD1(fatfs);
		if(sd_status == CMD_SUCCESS)
		{
			//SD1 is not the problem
		}
		else
		{
			//SD1 is the problem
			//set a flag to indicate to only use SD0?
			xil_printf("SD1 failed to mount\r\n");
		}
	}
	// *********** Initialize Mini-NS System Parameters ****************//
	InitConfig();

	// *********** Initialize Local Variables ****************//

	//start timing
	InitStartTime();

//	//This code is used for timing functions to get latency measurements
//	long long int time_holder = 0;
//	XTime timer1 = 0;//test timers, can delete
//	XTime timer2 = 0;

	// Initialize buffers
	char RecvBuffer[100] = "";

	char * last_command;				//pointer to handle writing commands to the log file
	unsigned int last_command_size = 0;	//holder for size
	int done = 0;						//local status variable for keeping track of progress within loops
	int status = 0;						//local status variable for reporting SUCCESS/FAILURE
	int DAQ_run_number = 0;				//run number value for file names, tracks the number of runs per POR
	int	menusel = 99999;				//case select variable for polling

	// ******************* APPLICATION LOOP *******************//

	//This loop will continue forever and the program won't leave it
	//This loop checks for input from the user, then checks to see if it's time to report SOH
	//if input is received, then it reads the input for correctness
	// if input is a valid MNS command, sends a command success packet
	// if not, then sends a command failure packet
	//after, SOH is checked to see if it is time to report SOH
	//When it is time, reports a full CCSDS SOH packet

	while(1){	//OUTER LEVEL 2 TESTING LOOP
		while(1){
			//resetting this value every time is (potentially) critical
			//resetting this ensures we don't re-use a command a second time (erroneously)
			menusel = 99999;
			menusel = ReadCommandType(RecvBuffer, &Uart_PS);	//Check for user input

			if ( menusel >= -1 && menusel <= 15 )	//let all input in, including errors, so we can report them
			{
				//we found a valid LUNAH command or input was bad (-1)
				//log the command issued, unless it is an error
				if(menusel != -1)
				{
					//write to the log file
					//get the command from ReadCommandType
					last_command = GetLastCommand();
					//get the size of the command
					last_command_size = GetLastCommandSize();
					//send the command to the log file write function
					LogFileWrite( last_command, last_command_size );
					//should probably just have the following syntax
					//LogFileWrite( GetLastCommand(), GetLastCommandSize() );
				}
				break;	//leave the inner loop and execute the commanded function
			}
			//check to see if it is time to report SOH information, 1 Hz
			CheckForSOH(&Iic, Uart_PS);
		}//END TEMP ASU TESTING LOOP

		//MAIN MENU OF FUNCTIONS
		switch (menusel) { // Switch-Case Menu Select
		case -1:
			//we found an invalid command
			//xil_printf("bad command detected\r\n");	//break and return to polling loop
			//Report CMD_FAILURE
			reportFailure(Uart_PS);
			break;
		case DAQ_CMD:
			//set processed data mode
			Xil_Out32(XPAR_AXI_GPIO_14_BASEADDR, 4);
			//prepare the status variables
			done = 0;				//not done yet
			status = CMD_SUCCESS;	//reset the variable so that we jump into the loop
			//create the file names we will use for this run:
			//check if the filename given is unique
			//if the filename is unique, then we will go through these functions once
			//if not, then we will loop until a unique name is found
			while(status == CMD_SUCCESS)
			{
				//only report a packet when the file has been successfully changed and did not exist already
				++DAQ_run_number;	//initialized to 0, the first run will increment to 1
				SetFileName(GetIntParam(1), DAQ_run_number, 0);	//creates a file name of IDNum_runNum_type.bin
				//check that the file name(s) do not already exist on the SD card...we do not want to append files
				status = DoesFileExist();
				//returns FALSE if file does NOT exist
				//returns TRUE if file does exist
				//we need the file to be unique, so FALSE is a positive result,
				// if we get TRUE, we need to keep looping
				//when status is FALSE, we need to send a packet to inform the file name
				if(status == CMD_FAILURE)
				{
					reportSuccess(Uart_PS, 1);
					//create the files before polling for user input
					//This also fills in the data header, as much as we know
					// at this point, we don't know the Real Time yet
					status = CreateDAQFiles();
				}
				//in case this takes longer, let's check for SOH in case this has to loop more than once
				//check to see if it is time to report SOH information, 1 Hz
				CheckForSOH(&Iic, Uart_PS);
			}

			//begin polling for START/BREAK/READTEMP
			while(done != 1)
			{
				status = ReadCommandType(RecvBuffer, &Uart_PS);	//Check for user input
				//see if we got anything meaningful //we'll accept any valid command
				if ( status >= -1 && status <= 23 )
				{
					//compare to what is accepted
					//if no good input is found, silently ignore the input
					switch(status){
					case -1:
						//we found an invalid command
						done = 0;	//continue looping //not done
						//Report CMD_FAILURE
						reportFailure(Uart_PS);
						break;
					case READ_TMP_CMD:
						//read all temp sensors
						done = 0;	//continue looping //not done
						//report a temp packet
						status = report_SOH(&Iic, GetLocalTime(), GetNeutronTotal(), Uart_PS, READ_TMP_CMD);
						if(status == CMD_FAILURE)
							reportFailure(Uart_PS);
						break;
					case BREAK_CMD:
						//received the break command
						//break out after this command //done
						done = 1;
						//report a success packet with the break command in it
						reportSuccess(Uart_PS, 0);
						break;
					case START_CMD:
						//received START_DAQ command
						//turn on the system
						//trigger a "false event" in the FPGA to log the MNS_FPGA time
						/***************for testing, leave these out, we don't need them */
	//					Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 1);	//enable capture module
	//					Xil_Out32(XPAR_AXI_GPIO_6_BASEADDR, 1);		//enable ADC
	//					Xil_Out32 (XPAR_AXI_GPIO_7_BASEADDR, 1);	//enable 5V to analog board
						//record the REALTIME and write headers into files
						status = WriteRealTime(GetRealTimeParam());
						//call the DAQ() function
						status = DataAcquisition(&Iic, Uart_PS, RecvBuffer);
						//we have returned from DAQ, report success/failure
						//we will return in three ways:
						// time out (1) = success
						// END (2)		= success
						// BREAK (0)	= failure

						//currently, this should always report failure because the function before it(write header files) always reports status = 0
						switch(status)
						{
						case 0:
							reportFailure(Uart_PS);
							break;
						case 1:
							reportSuccess(Uart_PS, 0);
							break;
						case 2:
							reportSuccess(Uart_PS, 0);
							break;
						default:
							reportFailure(Uart_PS);
							break;
						}
						//break out after this command //done
						done = 1;
						break;
					default:
						//got something outside of these commands
						done = 0;	//continue looping //not done
						//I want to report failure and also include something like daq loop in the string
						// that way a person controlling the MNS would see the reason all their commands
						// are failing. If the loop is here, the command entered was recognized and relevant
						// but we can't do it because this loop doesn't have access.
						reportFailure(Uart_PS);
						break;
					}
				}
				//check to see if it is time to report SOH information, 1 Hz
				CheckForSOH(&Iic, Uart_PS);
			}
			//data acquisition has been completed, wrap up anything not handled by the DAQ function

			break;
		case WF_CMD:
			//do WF acquisition here
			//this is level 3 stuff
			//xil_printf("received WF command\r\n");
			break;
		case READ_TMP_CMD:
			//tell the report_SOH function that we want a temp packet
			status = report_SOH(&Iic, GetLocalTime(), GetNeutronTotal(), Uart_PS, READ_TMP_CMD);
			if(status == CMD_FAILURE)
				reportFailure(Uart_PS);
			break;
		case GETSTAT_CMD: //Push an SOH packet to the bus
			//instead of checking for SOH, just push one SOH packet out because it was requested
			status = report_SOH(&Iic, GetLocalTime(), GetNeutronTotal(), Uart_PS, GETSTAT_CMD);
			if(status == CMD_FAILURE)
				reportFailure(Uart_PS);
			break;
		case DISABLE_ACT_CMD:
			//disable the components
			Xil_Out32(XPAR_AXI_GPIO_6_BASEADDR, 0);		//disable 3.3V
			Xil_Out32(XPAR_AXI_GPIO_7_BASEADDR, 0);		//disable 5v to Analog board
			//No SW check on success/failure
			//Report SUCCESS (no way to check for failure)
			reportSuccess(Uart_PS, 0);
			break;
		case ENABLE_ACT_CMD:
			//enable the active components
			Xil_Out32(XPAR_AXI_GPIO_6_BASEADDR, 1);		//enable ADC
			Xil_Out32(XPAR_AXI_GPIO_7_BASEADDR, 1);		//enable 5V to analog board
			//No SW check on success/failure
			//Report SUCCESS (no way to check for failure)
			reportSuccess(Uart_PS, 0);
			break;
		case TX_CMD:
			//transfer any file on the SD card
			//xil_printf("received TX command\r\n");
			break;
		case DEL_CMD:
			//delete a file from the SD card
			//xil_printf("received DEL command\r\n");
			break;
		case LS_CMD:
			//transfer the names and sizes of the files on the SD card
			//xil_printf("received LS_FILES command\r\n");
			break;
		case TXLOG_CMD:
			//transfer the system log file
			//xil_printf("received TXLOG command\r\n");
			break;
		case CONF_CMD:
			//transfer the configuration file
			//xil_printf("received CONF command\r\n");
			break;
		case TRG_CMD:
			//set the trigger threshold
			status = SetTriggerThreshold( GetIntParam(1) );
			//Determine SUCCESS or FAILURE
			if(status)
				reportSuccess(Uart_PS, 0);
			else
				reportFailure(Uart_PS);
			break;
		case ECAL_CMD:
			//set the energy calibration parameters
			status = SetEnergyCalParam( GetFloatParam(1), GetFloatParam(2) );
			//Determine SUCCESS or FAILURE
			if(status)
				reportSuccess(Uart_PS, 0);
			else
				reportFailure(Uart_PS);
			break;
		case NGATES_CMD:
			//set the neutron cuts
			status = SetNeutronCutGates(GetIntParam(1), GetIntParam(2), GetFloatParam(1), GetFloatParam(2), GetFloatParam(3), GetFloatParam(4) );
			//Determine SUCCESS or FAILURE
			if(status)
				reportSuccess(Uart_PS, 0);
			else
				reportFailure(Uart_PS);
			break;
		case HV_CMD:
			//set the PMT bias voltage for one or more PMTs
			//intParam1 = PMT ID
			//intParam2 = Bias Voltage (taps)
			status = SetHighVoltage(&Iic, GetIntParam(1), GetIntParam(2));
			//Determine SUCCESS or FAILURE
			if(status)
				reportSuccess(Uart_PS, 0);
			else
				reportFailure(Uart_PS);
			break;
		case INT_CMD:
			//set the integration times
			//intParam1 = Baseline integration time
			//intParam2 = Short integration time
			//intParam3 = Long integration time
			//intParam4 = Full integration time
			status = SetIntergrationTime(GetIntParam(1), GetIntParam(2), GetIntParam(3), GetIntParam(4));
			//Determine SUCCESS or FAILURE
			if(status)
				reportSuccess(Uart_PS, 0);
			else
				reportFailure(Uart_PS);
			break;
		case INPUT_OVERFLOW:
			//too much input
			//TODO: Handle this problem here and in ReadCommandType
//			xil_printf("Overflowed the buffer (too much input at one time)\r\n");
//			xil_printf("The buffer will try and read through what was sent\r\n");
			break;
		default:
			//got a value for menusel we did not expect
			//list of accepted values are found in "lunah_defines.h"
			//what is the list of values we can receive total?
			//xil_printf("something weird happened: default at main menu\r\n");
			//Report CMD_FAILURE
			break;
		}//END OF SWITCH/CASE (MAIN MENU OF FUNCTIONS)

		//check to see if it is time to report SOH information, 1 Hz
		//this may help with functions which take too long during their own loops
		CheckForSOH(&Iic, Uart_PS);
	}//END OF OUTER LEVEL 2 TESTING LOOP

/*********************************BELOW THIS IS OLD CODE*************************************
 *
 * There are a couple of functions related to Init which need to be retained. They are:
 *
 * InitializeAXIDma
 * InitializeInterruptSystem
 * InterruptHandler
 * SetUpInterruptSystem
 *
 * At some point, these functions will need to be moved to a separate file, likely to
 * the lunah_utils file.
 *
 * TODO: Figure out if we need the interrupt system code.
 * TODO: Move this code to the Lunah_utils source file
 *
 */

    return 0;
}

//////////////////////////// InitializeAXIDma////////////////////////////////
// Sets up the AXI DMA
int InitializeAXIDma(void) {
	u32 tmpVal_0 = 0;
	//u32 tmpVal_1 = 0;

	tmpVal_0 = Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x30);

	tmpVal_0 = tmpVal_0 | 0x1001; //<allow DMA to produce interrupts> 0 0 <run/stop>

	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x30, tmpVal_0);
	Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x30);	//what does the return value give us? What do we do with it?

	return 0;
}
//////////////////////////// InitializeAXIDma////////////////////////////////

//////////////////////////// InitializeInterruptSystem////////////////////////////////
int InitializeInterruptSystem(u16 deviceID) {
	int Status;
	static XScuGic_Config *GicConfig; 	// GicConfig
	XScuGic InterruptController;		// Interrupt controller

	GicConfig = XScuGic_LookupConfig (deviceID);

	if(NULL == GicConfig) {

		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	Status = SetUpInterruptSystem(&InterruptController);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	Status = XScuGic_Connect (&InterruptController,
			XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR,
			(Xil_ExceptionHandler) InterruptHandler, NULL);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;

	}

	XScuGic_Enable(&InterruptController, XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR );

	return XST_SUCCESS;

}
//////////////////////////// InitializeInterruptSystem////////////////////////////////


//////////////////////////// Interrupt Handler////////////////////////////////
void InterruptHandler (void ) {

	u32 global_frame_counter = 0;	// Counts for the interrupt system
	u32 tmpValue;
	tmpValue = Xil_In32(XPAR_AXI_DMA_0_BASEADDR + 0x34);
	tmpValue = tmpValue | 0x1000;
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x34, tmpValue);

	global_frame_counter++;
}
//////////////////////////// Interrupt Handler////////////////////////////////


//////////////////////////// SetUp Interrupt System////////////////////////////////
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr) {
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, XScuGicInstancePtr);
	Xil_ExceptionEnable();
	return XST_SUCCESS;

}
//////////////////////////// SetUp Interrupt System////////////////////////////////

//////////////////////////// get_data ////////////////////////////////
#ifndef BREAKUP_MAIN
int get_data(XUartPs * Uart_PS, char * EVT_filename0, char * CNT_filename0, char * EVT_filename1, char * CNT_filename1, int i_neutron_total, char * RecvBuffer, XTime local_time_start, XTime local_time)
#else
int get_data(XUartPs Uart_PS, char * EVT_filename0, char * CNT_filename0, char * EVT_filename1, char * CNT_filename1, char * RecvBuffer)
#endif
{
	uint numBytesWritten = 0;
	uint numBytesRead = 0;
	int valid_data = 0; 	//BRAM buffer size
	int buff_num = 0;	//keep track of which buffer we are writing
	int array_index = 0;
	int dram_addr;
	int dram_base = 0xa000000;
	int dram_ceiling = 0xA004000;
	int ipollReturn = 0;	//keep track of user input
	//2DH variables
//	int i_xnumbins = 260;
//	int i_ynumbins = 30;
	//buffers are 4096 ints long (512 events total)
	unsigned int * data_array;
	unsigned int * data_array_holder;
	data_array = (unsigned int *)malloc(sizeof(unsigned int)*DATA_BUFFER_SIZE*4);
	memset(data_array, '0', DATA_BUFFER_SIZE * sizeof(unsigned int)); //zero out the array
//	unsigned short twoDH_pmt1[i_xnumbins][i_ynumbins];
//	unsigned short twoDH_pmt2[i_xnumbins][i_ynumbins];
//	unsigned short twoDH_pmt3[i_xnumbins][i_ynumbins];
//	unsigned short twoDH_pmt4[i_xnumbins][i_ynumbins];

	//SD CARD FILES
	FIL data_file;
	FIL data_file_dest;
	FRESULT ffs_res;

	//timing
//	XTime local_time_current;
//	local_time_current = 0;

	XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);

	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000); 		// DMA Transfer Step 1
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);			// DMA Transfer Step 2
	sleep(1);
//	ClearBuffers();

	while(ipollReturn != 15 && ipollReturn != 17)	//DATA ACQUISITION LOOP
	{
		//check the buffer to see if we have valid data
		valid_data = Xil_In32 (XPAR_AXI_GPIO_11_BASEADDR);	// AA write pointer // tells how far the system has read in the AA module
		if(valid_data == 1)
		{
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);				// init mux to transfer data between integrater modules to DMA
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			usleep(54); 												// this will change
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0);

			Xil_DCacheInvalidateRange(0xa0000000, 65536);

			//prepare for looping
			array_index = 0;
			dram_addr = dram_base;
			switch(buff_num)
			{
			case 0:
				//fetch the data from the DRAM
				data_array_holder = data_array;
				while(dram_addr <= dram_ceiling)
				{
					data_array_holder[array_index] = Xil_In32(dram_addr);
					dram_addr+=4;
					array_index++;
				}
//				ProcessData(data_array_holder, twoDH_pmt1, twoDH_pmt2, twoDH_pmt3, twoDH_pmt4);
//				ClearBuffers();
				buff_num++;
				break;
			case 1:
				//fetch the data from the DRAM //data_array + 4096
				data_array_holder = data_array + DATA_BUFFER_SIZE; //move the pointer to the buffer
				while(dram_addr <= dram_ceiling)
				{
					data_array_holder[array_index] = Xil_In32(dram_addr);
					dram_addr+=4;
					array_index++;
				}
//				ProcessData(data_array_holder, twoDH_pmt1, twoDH_pmt2, twoDH_pmt3, twoDH_pmt4);
//				ClearBuffers();
				buff_num++;
				break;
			case 2:
				//fetch the data from the DRAM //data_array + 8192
				data_array_holder = data_array + DATA_BUFFER_SIZE*2;
				while(dram_addr <= dram_ceiling)
				{
					data_array_holder[array_index] = Xil_In32(dram_addr);
					dram_addr+=4;
					array_index++;
				}
//				ProcessData(data_array_holder, twoDH_pmt1, twoDH_pmt2, twoDH_pmt3, twoDH_pmt4);
//				ClearBuffers();
				buff_num++;
				break;
			case 3:
				//fetch the data from the DRAM //data_array + 12288
				data_array_holder = data_array + DATA_BUFFER_SIZE*3;
				while(dram_addr <= dram_ceiling)
				{
					data_array_holder[array_index] = Xil_In32(dram_addr);
					dram_addr+=4;
					array_index++;
				}
//				process_data(data_array_holder, &(twoDH_pmt1[0][0]), &(twoDH_pmt2[0][0]), &(twoDH_pmt3[0][0]), &(twoDH_pmt4[0][0]));
//				ClearBuffers();
				buff_num = 0;

				//write the event data to SD card
				ffs_res = f_open(&data_file, EVT_filename1, FA_WRITE|FA_READ|FA_OPEN_ALWAYS);	//open the file
				if(ffs_res)
					xil_printf("Could not open file %d\n", ffs_res);
				ffs_res = f_lseek(&data_file, file_size(&data_file));	//seek to the end of the file
				//write the data //4 buffers total // 512 events per buff
				ffs_res = f_write(&data_file, data_array, sizeof(u32)*4096*4, &numBytesWritten);
				ffs_res = f_close(&data_file);
				//write the cnt data to SD card
				ffs_res = f_open(&data_file, CNT_filename1, FA_WRITE|FA_READ|FA_OPEN_ALWAYS);	//open the file
				if(ffs_res)
					xil_printf("Could not open file %d\n", ffs_res);
				ffs_res = f_lseek(&data_file, file_size(&data_file));	//seek to the end of the file
				//write the data //4 buffers total // 512 events per buff
				ffs_res = f_write(&data_file, data_array, sizeof(u32)*4096*4, &numBytesWritten);
				ffs_res = f_close(&data_file);
				break;
			default:
				//how to deal if we have a weird value of buff_num?
				//treat the data like a singleton buffer and process and save it
				// in this way, we don't skip any data or overwrite anything we previously had
				//should check where the data_array pointer is at first
				xil_printf("buff_num in DAQ outside of bounds\r\n");
				break;
			}
		}

		//continue to loop and report SOH while waiting for user input
#ifdef BREAKUP_MAIN
//		CheckForSOH(Iic, Uart_PS);
#else
		XTime_GetTime(&local_time_current);
		if(((local_time_current - local_time_start)/COUNTS_PER_SECOND) >= (local_time +  1))
		{
			local_time = (local_time_current - local_time_start)/COUNTS_PER_SECOND;
			report_SOH(local_time, i_neutron_total, Uart_PS);
		}
#endif
		//check user input
		ipollReturn = ReadCommandType(RecvBuffer, &Uart_PS);
		switch(ipollReturn) {
		case 15:
			//BREAK was received
			break;
		case 17:
			//END was received
			break;
		case 100:
			//too much data in the receive buffer
			memset(RecvBuffer, '0', 100);
			break;
		case 999:
			//no line ending has been entered, continue with operation
			break;
		default:
			//anything else
			memset(RecvBuffer, '0', 100);
			XUartPs_Send(&Uart_PS, (u8 *)"FFFFFF\n", 7);
			break;
		}

	} //END DATA ACQUISITION LOOP

	//DUPLICATE the data files (CNT, EVT) onto SD 0
	ffs_res = f_open(&data_file, EVT_filename1, FA_READ);
	ffs_res = f_open(&data_file_dest, EVT_filename0, FA_WRITE | FA_OPEN_ALWAYS);

	for(;;)
	{
		ffs_res = f_read(&data_file, data_array, sizeof(u32)*4096*4, &numBytesRead);
		if(ffs_res || numBytesRead == 0)
			break;
		ffs_res = f_write(&data_file_dest, data_array, numBytesRead, &numBytesWritten);
		if(ffs_res || numBytesWritten < numBytesRead)
			break;
	}

	f_close(&data_file);
	f_close(&data_file_dest);

	ffs_res = f_open(&data_file, CNT_filename1, FA_READ);
	ffs_res = f_open(&data_file_dest, CNT_filename0, FA_WRITE | FA_OPEN_ALWAYS);

	for(;;)
	{
		ffs_res = f_read(&data_file, data_array, sizeof(u32)*4096*4, &numBytesRead);
		if(ffs_res || numBytesRead == 0)
			break;
		ffs_res = f_write(&data_file_dest, data_array, numBytesRead, &numBytesWritten);
		if(ffs_res || numBytesWritten < numBytesRead)
			break;
	}

	f_close(&data_file);
	f_close(&data_file_dest);

	//Done duplicating files onto backup SD card

	free(data_array);
//  return i_neutron_total;
	return(GetNeutronTotal());  // is there a reasons to return neuron total will if changed it will be with PutNeuronTotal in lunah_utils.c
}

//////////////////////////// get_data ////////////////////////////////
