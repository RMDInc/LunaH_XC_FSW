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
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include "main.h"


//this is Flight Software Version 2.1
//Updated 8-17-2018, 12:08

//struct info
struct header_info{
	long long int real_time;
	int baseline_int;
	int short_int;
	int long_int;
	int full_int;
	short orbit_number;
	short run_number;
	unsigned char file_type;
};

int main()
{
    // Initialize System
	ps7_post_config();
	Xil_DCacheDisable();	// Disable the data cache
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
	//IIC0 = sensor head
	//IIC1 = thermometer
	//XIicPs Iic;

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
	// *********** Mount SD Card and Initialize Variables ****************//
	if( doMount == 0 ){			//Initialize the SD card here
		ffs_res = f_mount(NULL,"0:/",0);
		ffs_res = f_mount(&fatfs[0], "0:/",0);
		ffs_res = f_mount(NULL,"1:/",0);
		ffs_res = f_mount(&fatfs[1],"1:/",0);
		doMount = 1;
	}
	if( f_stat( cLogFile, &fno) ){	// f_stat returns non-zero(false) if no file exists, so open/create the file
		ffs_res = f_open(&logFile, cLogFile, FA_WRITE|FA_OPEN_ALWAYS);
		ffs_res = f_write(&logFile, cZeroBuffer, 10, &numBytesWritten);
		filptr_clogFile += numBytesWritten;		// Protect the first xx number of bytes to use as flags, in this case xx must be 10
		ffs_res = f_close(&logFile);
	}
	else // If the file exists, read it
	{
		ffs_res = f_open(&logFile, cLogFile, FA_READ|FA_WRITE);	//open with read/write access
		ffs_res = f_lseek(&logFile, 0);							//go to beginning of file
		ffs_res = f_read(&logFile, &filptr_buffer, 10, &numBytesRead);	//Read the first 10 bytes to determine flags and the size of the write pointer
		sscanf(filptr_buffer, "%d", &filptr_clogFile);			//take the write pointer from char -> integer so we may use it
		ffs_res = f_lseek(&logFile, filptr_clogFile);			//move the write pointer so we don't overwrite info
		iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "POWER RESET %f ", dTime);	//write that the system was power cycled
		ffs_res = f_write(&logFile, cWriteToLogFile, iSprintfReturn, &numBytesWritten);	//write to the file
		filptr_clogFile += numBytesWritten;						//update the write pointer
		ffs_res = f_close(&logFile);							//close the file
	}

	InitConfig();

	//we are going to skip this code and not write a directory log file
	// because we can use an SD card function to write out the contents
	// of the card later
//	if( f_stat(cDirectoryLogFile0, &fnoDIR) )	//check if the file exists
//	{
//		ffs_res = f_open(&directoryLogFile, cDirectoryLogFile0, FA_WRITE|FA_OPEN_ALWAYS);	//if no, create the file
//		ffs_res = f_write(&directoryLogFile, cZeroBuffer, 10, &numBytesWritten);			//write the zero buffer so we can keep track of the write pointer
//		filptr_cDIRFile += 10;																//move the write pointer
//		ffs_res = f_write(&directoryLogFile, cLogFile, 11, &numBytesWritten);				//write the name of the log file because it was created above
//		filptr_cDIRFile += numBytesWritten;													//update the write pointer
//		snprintf(cWriteToLogFile, 10, "%d", filptr_cDIRFile);								//write formatted output to a sized buffer; create a string of a certain length
//		ffs_res = f_lseek(&directoryLogFile, (10 - LNumDigits(filptr_cDIRFile)));			// Move to the start of the file
//		ffs_res = f_write(&directoryLogFile, cWriteToLogFile, LNumDigits(filptr_cDIRFile), &numBytesWritten);	//Record the new file pointer
//		ffs_res = f_close(&directoryLogFile);												//close the file
//	}
//	else	//if the file exists, read it
//	{
//		ffs_res = f_open(&directoryLogFile, cDirectoryLogFile0, FA_READ);					//open the file
//		ffs_res = f_lseek(&directoryLogFile, 0);											//move to the beginning of the file
//		ffs_res = f_read(&directoryLogFile, &filptr_cDIRFile_buffer, 10, &numBytesWritten);	//read the write pointer
//		sscanf(filptr_cDIRFile_buffer, "%d", &filptr_cDIRFile);								//write the pointer to the relevant variable
//		ffs_res = f_close(&directoryLogFile);												//close the file
//	}
	// *********** Mount SD Card and Initialize Variables ****************//

	//start timing
	XTime local_time_start = 0;
//	XTime local_time_current = 0;
	XTime local_time = 0;
	XTime_GetTime(&local_time_start);	//get the time
	InitStartTime();
/*
	long long int time_holder = 0;
	XTime timer1 = 0;//test timers, can delete
	XTime timer2 = 0;
*/
	//Temp/I2C variables
	int a, b;					//used for bitwise operations
	int analog_board_temp = 0;
	int digital_board_temp = 0;

	// Initialize buffers
	char filename_buff[50] = "";
	char full_filename_buff[54] = "";
	char report_buff[100] = "";
	char c_CNT_Filename0[FILENAME_SIZE] = "";
	char c_EVT_Filename0[FILENAME_SIZE] = "";
	char c_CNT_Filename1[FILENAME_SIZE] = "";
	char c_EVT_Filename1[FILENAME_SIZE] = "";
	char c_file_to_TX[FILENAME_SIZE] = "";
	char c_file_to_access[FILENAME_SIZE] = "";
	char transfer_file_contents[DATA_PACKET_SIZE] = "";
	char RecvBuffer[100] = "";
	/* move to make global
	unsigned char error_buff[] = "FFFFFF\n";
	unsigned char break_buff[] = "FAFAFA\n";
	unsigned char success_buff[] = "AAAAAA\n"; */
	int ipollReturn = 0;
	int	menusel = 99999;	// Menu Select
	int i_sscanf_ret = 0;
	int i_orbit_number = 0;
	int i_daq_run_number = 0;
	int status = 0;
	int iterator = 0;
	int sent = 0;
	int bytes_sent = 0;
	int bytes_recvd = 0;
	int file_size = 0;
	int checksum1 = 0;
	int checksum2 = 0;
	int i_neutron_total = 0;
	int i_trigger_threshold = 0;	// Trigger Threshold
	int *IIC_SLAVE_ADDR;		//pointer to slave
	unsigned int sync_marker = 0x352EF853;
	unsigned long long int i_real_time = 0;
	int i_integration_times[4] = {};
	float ECut_1;
	float PCut_1;
	float ECut_2;
	float PCut_2;
	int HvPmtId, HvValue;

	struct header_info file_header = {};

	// ******************* APPLICATION LOOP *******************//

	//temporary while loop to use for ASU testing on 10/15/2018
	//This loop will continue forever and the program won't leave it
	//This loop checks for input from the user, then checks to see if it's time to report SOH
	//if input is received, then it reads the input for correctness
	// if input is a valid LUNAH command, sends a command success packet
	// if not, then sends a command failure packet
	//after, SOH is checked to see if it is time to report SOH
	//When it is time, reports a full CCSDS SOH packet
	while(1){	//BEGIN TEMP ASU TESTING LOOP
		memset(RecvBuffer, '0', 100);	// Clear RecvBuffer Variable
		XUartPs_SetOptions(&Uart_PS, XUARTPS_OPTION_RESET_RX);
		iPollBufferIndex = 0;

		while(1)	//POLLING LOOP // MAIN MENU
		{
			menusel = 99999;
			menusel = ReadCommandType(RecvBuffer, &Uart_PS);
			menusel = 1;
//			xil_printf("Menusel:");
//			xil_printf("%d\n", menusel);

			if ( menusel > -1 && menusel <= 18 ){
				//we found a valid LUNAH command
				//report command success

				char *p = reportSuccess(menusel);
				if(p) {
//					xil_printf("Command success:");
					xil_printf("%s\n", p);
					memset(p, 0, sizeof(p));
				}

				char *q = reportFailure(menusel);
				if(q) {
//					xil_printf("Command failure:");
					xil_printf("%s\n", q);
					memset(q, 0, sizeof(q));
				}

				break;
			}
			else if( menusel == -1){
				//we found an invalid LUNAH command
				//report command failure

				break;
			}

			//until we have found something useful, check to see if we need
			// to report SOH information, 1 Hz
			CheckForSOH(Uart_PS);
		}//END POLLING LOOP //END MAIN MENU

		ipollReturn = 999;
	}//END TEMP ASU TESTING LOOP

	while(1){
		memset(RecvBuffer, '0', 100);	// Clear RecvBuffer Variable
		XUartPs_SetOptions(&Uart_PS, XUARTPS_OPTION_RESET_RX);
		iPollBufferIndex = 0;

		while(1)	//POLLING LOOP // MAIN MENU
		{
			menusel = 99999;
			menusel = ReadCommandType(RecvBuffer, &Uart_PS);

			if ( menusel >= -1 && menusel <= 18 )
				break;

			//until we have found something useful, check to see if we need
			// to report SOH information, 1 Hz
			CheckForSOH(Uart_PS);
		}//END POLLING LOOP //END MAIN MENU

		ipollReturn = 999;
		switch (menusel) { // Switch-Case Menu Select
		case -1:
			//we found an invalid LunaH command
			//report command failure

			break;
		case DAQ__CMD: //DAQ mode
#ifdef BREAKUP_MAIN_DAQ
			i_sscanf_ret = sscanf(RecvBuffer + 3 + 1, " %d", &i_orbit_number);
			DataAcqInit(menusel, i_orbit_number);
			break;
#else

			//can check this later to see if we are coming from the a new DAQ call
			// or if we are trying to jump back in during a run
			i_sscanf_ret = sscanf(RecvBuffer + 3 + 1, " %d", &i_orbit_number);
			//increment DAQ run number each time we make it to this function
			i_daq_run_number++;
			//set processed data mode
			Xil_Out32(XPAR_AXI_GPIO_14_BASEADDR, 4);

			//create files and pass in filename to getWFDAQ function
			snprintf(c_EVT_Filename0, 50, "0:/%04d_%04d_evt.bin",i_orbit_number, i_daq_run_number);	//assemble the filename
			snprintf(c_CNT_Filename0, 50, "0:/%04d_%04d_cnt.bin",i_orbit_number, i_daq_run_number);
			//2dh file here
			snprintf(c_EVT_Filename1, 50, "1:/%04d_%04d_evt.bin",i_orbit_number, i_daq_run_number);
			snprintf(c_CNT_Filename1, 50, "1:/%04d_%04d_cnt.bin",i_orbit_number, i_daq_run_number);
			//2dh file here
			iSprintfReturn = snprintf(report_buff, 100, "%s_%s\n", c_EVT_Filename0, c_CNT_Filename0);	//create the string to tell CDH

			//poll for START, BREAK
			memset(RecvBuffer, '0', 100);
			iPollBufferIndex = 0;
			while(ipollReturn != BREAK_CMD && ipollReturn != START_CMD)
			{
				ipollReturn = ReadCommandType(RecvBuffer, &Uart_PS);
				switch(ipollReturn) {
				case BREAK_CMD:
					//BREAK was received
					break;
				case START_CMD:
					//START was received, get the real time from the S/C
					sscanf(RecvBuffer + 5 + 1, " %llu", &i_real_time);
					Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 1);	//enable capture module
					//the above triggers a false event which we use to sync the S/C time
					// with the local FPGA time
					Xil_Out32(XPAR_AXI_GPIO_6_BASEADDR, 1);		//enable ADC
					Xil_Out32 (XPAR_AXI_GPIO_7_BASEADDR, 1);	//enable 5V to analog board
					break;
				case 100:
					//too much input, clear buffer
					memset(RecvBuffer, '0', 100);
					break;
				case 999:
					//continue with operation, no input to process
					break;
				default:
					//anything else
					memset(RecvBuffer, '0', 100);
					bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)error_buff, sizeof(error_buff));
					break;
				}

				//continue to loop and report SOH while waiting for user input
				CheckForSOH(Uart_PS);

			}//END POLL UART

			//if BREAK, leave the loop //nothing to turn off
			if(ipollReturn == BREAK_CMD)
			{
				bytes_sent = XUartPs_Send(&Uart_PS, break_buff, sizeof(break_buff));
				break;
			}

			//write to the log file

			//write the headers and create the data files as listed above //this is the Mini-NS Data Header
			file_header.real_time = i_real_time;
			file_header.orbit_number = i_orbit_number;
			file_header.run_number = i_daq_run_number;
			file_header.baseline_int = Xil_In32(XPAR_AXI_GPIO_0_BASEADDR);
			file_header.short_int = Xil_In32(XPAR_AXI_GPIO_1_BASEADDR);
			file_header.long_int = Xil_In32(XPAR_AXI_GPIO_2_BASEADDR);
			file_header.full_int = Xil_In32(XPAR_AXI_GPIO_3_BASEADDR);

			//write the file to SD Card 0
			file_header.file_type = 0;
			ffs_res = f_open(&data_file, c_CNT_Filename0, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res)
				xil_printf("%d could not open cnt file\n", ffs_res);
			ffs_res = f_lseek(&data_file, file_size(&data_file));
			ffs_res = f_write(&data_file, &file_header, sizeof(file_header), &numBytesWritten);	//write the struct
			ffs_res = f_close(&data_file);

			file_header.file_type = 2;
			ffs_res = f_open(&data_file, c_EVT_Filename0, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res)
				xil_printf("%d could not open evt file\n", ffs_res);
			ffs_res = f_lseek(&data_file, file_size(&data_file));
			ffs_res = f_write(&data_file, &file_header, sizeof(file_header), &numBytesWritten);
			ffs_res = f_close(&data_file);

			//write the file to SD Card 1
			//create the file names
			file_header.file_type = 0;
			ffs_res = f_open(&data_file, c_CNT_Filename1, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res)
				xil_printf("could not open cnt file\n");
			ffs_res = f_lseek(&data_file, file_size(&data_file));
			ffs_res = f_write(&data_file, &file_header, sizeof(file_header), &numBytesWritten);	//write the struct
			ffs_res = f_close(&data_file);

			file_header.file_type = 2;
			ffs_res = f_open(&data_file, c_EVT_Filename1, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res)
				xil_printf("could not open evt file\n");
			ffs_res = f_lseek(&data_file, file_size(&data_file));
			ffs_res = f_write(&data_file, &file_header, sizeof(file_header), &numBytesWritten);
			ffs_res = f_close(&data_file);

			memset(RecvBuffer, '0', 100);
			iPollBufferIndex = 0;
			ClearBuffers();
			ipollReturn = 999;	//reset the variable

			//go to the DAQ function
			//go into the get_data function (renamed from GetWFDAQ())
#ifdef BREAKUP_MAIN
			get_data(Uart_PS, c_EVT_Filename0, c_CNT_Filename0, c_EVT_Filename1, c_CNT_Filename1, RecvBuffer);
#else
			get_data(&Uart_PS, c_EVT_Filename0, c_CNT_Filename0, c_EVT_Filename1, c_CNT_Filename1, i_neutron_total, RecvBuffer, local_time_start, local_time);
#endif
			//Broke out of the DAQ loop, finalize the run
			Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 0);	//enable capture module
			Xil_Out32(XPAR_AXI_GPIO_6_BASEADDR, 0);		//enable ADC
			Xil_Out32 (XPAR_AXI_GPIO_7_BASEADDR, 0);	//enable 5V to analog board

			//if BREAK, leave right away
			if(ipollReturn == 15)
			{
				bytes_sent = XUartPs_Send(&Uart_PS, break_buff, sizeof(break_buff));
				break;
			}

			//otherwise END was how we broke out and we should just finish things up
			sscanf(RecvBuffer + 3 + 1, " %llu", &i_real_time); //get the end time from the user or S/C

			//write the footer for the files
			ffs_res = f_open(&data_file, c_CNT_Filename0, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res)
				xil_printf("%d could not open cnt file\n", ffs_res);
			ffs_res = f_lseek(&data_file, file_size(&data_file));
			ffs_res = f_write(&data_file, &i_real_time, sizeof(i_real_time), &numBytesWritten);	//write the struct
			ffs_res = f_close(&data_file);

			ffs_res = f_open(&data_file, c_EVT_Filename0, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res)
				xil_printf("%d could not open evt file\n", ffs_res);
			ffs_res = f_lseek(&data_file, file_size(&data_file));
			ffs_res = f_write(&data_file, &i_real_time, sizeof(i_real_time), &numBytesWritten);
			ffs_res = f_close(&data_file);

			//write to SD Card 1
			ffs_res = f_open(&data_file, c_CNT_Filename1, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res)
				xil_printf("could not open cnt file\n");
			ffs_res = f_lseek(&data_file, file_size(&data_file));
			ffs_res = f_write(&data_file, &i_real_time, sizeof(i_real_time), &numBytesWritten);	//write the struct
			ffs_res = f_close(&data_file);

			ffs_res = f_open(&data_file, c_EVT_Filename1, FA_OPEN_ALWAYS | FA_WRITE);
			if(ffs_res)
				xil_printf("could not open evt file\n");
			ffs_res = f_lseek(&data_file, file_size(&data_file));
			ffs_res = f_write(&data_file, &i_real_time, sizeof(i_real_time), &numBytesWritten);
			ffs_res = f_close(&data_file);

			//write to log file
			//write the end of run neutron totals
			iSprintfReturn = snprintf(report_buff, LOG_FILE_BUFF_SIZE, "%u\n", i_neutron_total);	//return value for END_
			bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);
			break;
#endif
		case WF_CMD: //Capture WF data

			break;
		case TMP_CMD: //TMP Set Loop

			break;
		case GETSTAT_CMD: //Getstat
			//calculate the time
			local_time = GetLocalTime();
			//instead of checking for SOH, just push one SOH packet out because it was requested
			report_SOH(local_time, GetNeuronTotal(), Uart_PS);
			break;
		case DISABLE_ACT_CMD: //Disable Data Acquisition Components
			//this is the power (5v and 3.3v) to the Analog board
			Xil_Out32(XPAR_AXI_GPIO_18_BASEADDR, 0);	//disable system (disable capture module)
			Xil_Out32(XPAR_AXI_GPIO_6_BASEADDR, 0);		//disable 3.3V
			Xil_Out32(XPAR_AXI_GPIO_7_BASEADDR, 0);		//disable 5v to Analog board

			//write to log file
			bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)success_buff, sizeof(success_buff));
			break;
		case 5: //Disable TEC Bleed
			Xil_Out32(XPAR_AXI_GPIO_5_BASEADDR, 0);
			XGpioPs_WritePin(&Gpio, TEC_PIN, 0);

			//write to log file
			bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)success_buff, sizeof(success_buff));
			break;
		case 6:	//Enable TEC Bleed
			Xil_Out32(XPAR_AXI_GPIO_5_BASEADDR, 1);
			XGpioPs_WritePin(&Gpio, TEC_PIN, 1);

			//write to log file
			bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)success_buff, sizeof(success_buff));
			break;
		case 7:	//TX Files
			//use this case to send files from the SD card to the screen
			sscanf(RecvBuffer + 2 + 1, " %s", c_file_to_TX);	//read in the name of the file to be transmitted
			iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "file TX %s ", c_file_to_TX);
			//Write to log file

			//have the file name to TX, prepare it for sending
			ffs_res = f_open(&logFile, c_file_to_TX, FA_READ);	//open the file to transfer
			if(ffs_res != FR_OK)
			{
				bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)error_buff, sizeof(error_buff));
				break;
			}

			//init the CCSDS packet with some parts of the header
			memset(transfer_file_contents, 0, DATA_PACKET_SIZE);
			transfer_file_contents[0] = sync_marker >> 24; //sync marker
			transfer_file_contents[1] = sync_marker >> 16;
			transfer_file_contents[2] = sync_marker >> 8;
			transfer_file_contents[3] = sync_marker >> 0;
			transfer_file_contents[4] = 43;	//7:5 CCSDS version
			transfer_file_contents[5] = 42;	//APID lsb (0x200-0x3ff)
			transfer_file_contents[6] = 41;	//7:6 grouping flags//5:0 seq. count msb
			transfer_file_contents[7] = 0;	//seq. count lsb

			ffs_res = f_lseek(&logFile, 0);	//seek to the beginning of the file
			while(1)
			{
				//loop over the data in the file
				//read in one payload_max amount of data to our packet
				ffs_res = f_read(&logFile, (void *)&(transfer_file_contents[10]), PAYLOAD_MAX_SIZE, &numBytesRead);

				//packet size goes in byte 8-9
				//value is num bytes after CCSDS header minus one
				// so, data size + 2 checksums - 1 (for array indexing)
				file_size = numBytesRead + 2 - 1;
				transfer_file_contents[8] = file_size >> 8;
				transfer_file_contents[9] = file_size;

				//calculate simple and Fletcher checksums
				while(iterator < numBytesRead)
				{
					checksum1 = (checksum1 + transfer_file_contents[iterator + 10]) % 255;	//simple
					checksum2 = (checksum1 + checksum2) % 255;
					iterator++;
				}

				//save the checksums
				transfer_file_contents[file_size + 9] = checksum2;
				transfer_file_contents[file_size + 9 + 1] = checksum1;

				//we have filled up the packet header, send the data
				sent = 0;
				bytes_sent = 0;
				while(sent < (file_size + 11))
				{
					bytes_sent = XUartPs_Send(&Uart_PS, &(transfer_file_contents[0]) + sent, (file_size + 11) - sent);
					sent += bytes_sent;
				}

				if(numBytesRead < PAYLOAD_MAX_SIZE)
					break;
				else
					transfer_file_contents[7]++;
			} //end of while TX loop

			ffs_res = f_close(&logFile);
			break;
		case 8:	//DEL Files
			//use this case to delete files from the SD card
			sscanf(RecvBuffer + 3 + 1, " %s", c_file_to_access);	//read in the name of the file to be deleted
			iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "file DEL %s ", c_file_to_access);
			//WriteToLogFile

			if(!f_stat(c_file_to_access, &fno))	//check if the file exists on disk
			{
				ffs_res = f_unlink(c_file_to_access);	//try to delete the file
				if(ffs_res == FR_OK)
				{
					//successful delete
					iSprintfReturn = snprintf(report_buff, 100, "%d_AAAA\n", ffs_res);
					bytes_sent = XUartPs_Send(&Uart_PS,(u8 *)report_buff, iSprintfReturn);
				}
				else
				{
					//failed to delete
					bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)error_buff, sizeof(error_buff));
				}
			}
			else
			{
				//file did not exist
				bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)error_buff, sizeof(error_buff));
			}
			break;
		case 9: //LS_Files
			break;
		case TRG_CMD: //Trigger threshold set
#ifdef BREAKUP_MAIN
			sscanf(RecvBuffer + 3 + 1," %d", &i_trigger_threshold);	//read in value from the recvBuffer
			bytes_sent = SetTriggerThreshold(i_trigger_threshold);
			break;
#else
			sscanf(RecvBuffer + 3 + 1," %d", &i_trigger_threshold);	//read in value from the recvBuffer
			if((i_trigger_threshold > 0) && (i_trigger_threshold < 16000))				//check that it's within accepted values
			{
				Xil_Out32(XPAR_AXI_GPIO_10_BASEADDR, (u32)(i_trigger_threshold));
				iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Set trigger threshold to %d ", i_trigger_threshold);
				//write to log file

				//read back value from the FPGA and echo to user
				i_trigger_threshold = 0;	//reset var before reading
				i_trigger_threshold = Xil_In32(XPAR_AXI_GPIO_10_BASEADDR);
				iSprintfReturn = snprintf(report_buff, 100, "%d\n", i_trigger_threshold);
				bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);
			}
			else
				bytes_sent = XUartPs_Send(&Uart_PS, error_buff, sizeof(error_buff));
			break;
#endif
		case NGATES_CMD: //NGATES, set neutron cut gates
			sscanf(RecvBuffer + strlen("NGATES_"), " %f_%f_%f_%f", &(ECut_1), &(ECut_2), &(PCut_1), &(PCut_2));
			SetNeutronCutGates(ECut_1, ECut_2, PCut_1, PCut_2);
			break;
		case HV_CMD:
#ifdef BREAKUP_MAIN
			sscanf(RecvBuffer + strlen("HV_"), " %d_%d", &HvPmtId, &HvValue);
			SetHighVoltage(HvPmtId, HvValue);
			break;
#else
			//TEC startup -> TEC control enable -> TEC off //Set High Voltage
			//change VTSET before getting in to temp loop
			Xil_Out32 (XPAR_AXI_GPIO_4_BASEADDR, 1);	//TEC Control Enable//signal to regulator
			XGpioPs_WritePin(&Gpio, TEC_PIN, 1);		//TEC Startup//signal to controller
			xil_printf("TEC ON, Control Enable ON\r\n");

			//poll for user value of a temperature to move to
			xil_printf("Enter a number of taps to move the wiper to:\r\n");
//			ReadCommandPoll();
			data = 0;	//reset
			menusel = 99999;
			sscanf(RecvBuffer,"%d",&data);
			if( data < 0 || data > 255) { xil_printf("Invalid number of taps.\n\r"); sleep(1); break; }

			//poll for a time to wait for the temperature to settle
			xil_printf("Enter a timeout value to wait for, in seconds\r\n");
//			ReadCommandPoll();
			timeout = 0;
			sscanf(RecvBuffer,"%d",&timeout);
			if( timeout < 1 ) { xil_printf("Invalid timeout value\n\r"); sleep(1); break; }

			IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR4;
			cntrl = 0x2;	//write to RDAC
			i2c_Send_Buffer[0] = cntrl;
			i2c_Send_Buffer[1] = data;
			Status = IicPsMasterSend(IIC_DEVICE_ID_1, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);

			//VTSET has been changed, now loop while checking the temperature sensors
			// and user input to see if we should stop
			xil_printf("Press 'q' to stop\n\r");
			//set variables we'll need when looping
			IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR5;
			i2c_Send_Buffer[0] = 0x0;
			i2c_Send_Buffer[1] = 0x0;
			//take the time so we know how long to run for
			XTime_GetTime(&local_time_current);	//take the time in cycles
			local_time = (local_time_current - local_time_start)/COUNTS_PER_SECOND; //compute the time in seconds
			while(1)
			{
				//check timeout to see if we should be done
				XTime_GetTime(&local_time_current);
				if(((local_time_current - local_time_start)/COUNTS_PER_SECOND) >= (local_time +  timeout))
					break;

				//check user input
				bytes_received = XUartPs_Recv(&Uart_PS, (u8 *)RecvBuffer, 100);
				if ( RecvBuffer[0] == 'q' )
					break;
				else
					memset(RecvBuffer, '0', 100);

				//check temp sensor(s)
				//read the temperature from the analog board temp sensor, data sheet Extra Temp Sensor Board (0x4A)
				Status = IicPsMasterSend(IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				Status = IicPsMasterRecieve(IIC_DEVICE_ID_0, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
				a = i2c_Recv_Buffer[0]<< 5;
				b = a | i2c_Recv_Buffer[1] >> 3;
				if(i2c_Recv_Buffer[0] >= 128)
					b = (b - 8192) / 16;
				else
					b = b / 16;
				//check to see if the temp reported is the temp requested by the user
				//if it is, break out
				//check temp is not outside of acceptable range
				if(b > 50 || b < 0)
					break;
			}

			XGpioPs_WritePin(&Gpio, TEC_PIN, 0);		//TEC Startup disable
			Xil_Out32 (XPAR_AXI_GPIO_4_BASEADDR, 0);	//TEC Control disable
			xil_printf("TEC OFF, Control Enable OFF\n\r");
			sleep(1); //built in latency
			break;
#endif
		case 13: //Set Integration Times
#ifdef BREAKUP_MAIN
			sscanf(RecvBuffer + 3 + 1, " %d_%d_%d_%d", &i_integration_times[0], &i_integration_times[1], &i_integration_times[2], &i_integration_times[3]);
			bytes_sent = SetIntergrationTime(i_integration_times[0], i_integration_times[1], i_integration_times[2], i_integration_times[3]);
			break;
#else
			sscanf(RecvBuffer + 3 + 1, " %d_%d_%d_%d", &i_integration_times[0], &i_integration_times[1], &i_integration_times[2], &i_integration_times[3]);

			if((i_integration_times[0] < i_integration_times[1]) && ( i_integration_times[1] < i_integration_times[2]) && (i_integration_times[2] < i_integration_times[3]))	//if each is greater than the last
			{
				Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, ((u32)(i_integration_times[0]+52)/4));
				Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, ((u32)(i_integration_times[1]+52)/4));
				Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, ((u32)(i_integration_times[2]+52)/4));
				Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, ((u32)(i_integration_times[3]+52)/4));

				//write this to the log file
				iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Set integration times to %d %d %d %d ", i_integration_times[0], i_integration_times[1], i_integration_times[2], i_integration_times[3]);
				//write to log file function

				//send return value for function
				iSprintfReturn = snprintf(report_buff, 100, "%d_%d_%d_%d\n",i_integration_times[0], i_integration_times[1], i_integration_times[2], i_integration_times[3]);
				bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);
			}
			else
				bytes_sent = XUartPs_Send(&Uart_PS, error_buff, sizeof(error_buff));
			break;
#endif
		case 14:	//Read Temp on the digital board
			IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR2;
			i2c_Send_Buffer[0] = 0x0;
			i2c_Send_Buffer[1] = 0x0;
			Status = IicPsMasterSend(IIC_DEVICE_ID_1, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
			Status = IicPsMasterRecieve(IIC_DEVICE_ID_1, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
			a = i2c_Recv_Buffer[0]<< 5;
			b = a | i2c_Recv_Buffer[1] >> 3;
			if(i2c_Recv_Buffer[0] >= 128)
				b = (b - 8192) / 16;
			else
				b = b / 16;
			xil_printf("%d\xf8\x43\n\r", b); //take integer, which is in degrees C \xf8 = degree symbol, \x43 = C
			sleep(1); //built in latency
			break;
		case 15:
			//read the temperature from the analog board temp sensor
			IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR3;
			i2c_Send_Buffer[0] = 0x0;
			i2c_Send_Buffer[1] = 0x0;
			Status = IicPsMasterSend(IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
			Status = IicPsMasterRecieve(IIC_DEVICE_ID_0, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
			a = i2c_Recv_Buffer[0]<< 5;
			b = a | i2c_Recv_Buffer[1] >> 3;
			if(i2c_Recv_Buffer[0] >= 128)
				b = (b - 8192) / 16;
			else
				b = b / 16;
			xil_printf("%d\xf8\x43\n\r", b);
			sleep(1); //built in latency
			break;
		case 16:
			//read the temperature from the Extra Temp Sensor Board
			IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR5;
			i2c_Send_Buffer[0] = 0x0;
			i2c_Send_Buffer[1] = 0x0;
			Status = IicPsMasterSend(IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
			Status = IicPsMasterRecieve(IIC_DEVICE_ID_0, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
			a = i2c_Recv_Buffer[0]<< 5;
			b = a | i2c_Recv_Buffer[1] >> 3;
			if(i2c_Recv_Buffer[0] >= 128)
				b = (b - 8192) / 16;
			else
				b = b / 16;
			xil_printf("%d\xf8\x43\n\r", b);
			sleep(1); //built in latency
			break;
		default :
			break;
		} // End Switch-Case Menu Select

	}	// ******************* POLLING LOOP *******************//

	ffs_res = f_mount(NULL,"0:/",0);
	ffs_res = f_mount(NULL,"1:/",0);
    cleanup_platform();
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

#ifndef BREAKUP_MAIN
//////////////////////////// Report SOH Function ////////////////////////////////
//This function takes in the number of neutrons currently counted and the local time
// and pushes the SOH data product to the bus over the UART
int report_SOH(XTime local_time, int i_neutron_total, XUartPs Uart_PS)
{
	//Variables
	char report_buff[100] = "";
	unsigned char i2c_Send_Buffer[2] = {};
	unsigned char i2c_Recv_Buffer[2] = {};
	int a = 0;
	int b = 0;
	int analog_board_temp = 0;
	int digital_board_temp = 0;
	int i_sprintf_ret = 0;
	int *IIC_SLAVE_ADDR;		//pointer to slave

	//analog board temp - case 14
	IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR3;
	i2c_Send_Buffer[0] = 0x0;
	i2c_Send_Buffer[1] = 0x0;
	IicPsMasterSend(IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
	IicPsMasterRecieve(IIC_DEVICE_ID_0, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
	a = i2c_Recv_Buffer[0]<< 5;
	b = a | i2c_Recv_Buffer[1] >> 3;
	if(i2c_Recv_Buffer[0] >= 128)
		b = (b - 8192) / 16;
	else
		b = b / 16;
	analog_board_temp = b;

	//digital board temp - case 13
	IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR2;
	IicPsMasterSend(IIC_DEVICE_ID_1, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
	IicPsMasterRecieve(IIC_DEVICE_ID_1, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
	a = i2c_Recv_Buffer[0]<< 5;
	b = a | i2c_Recv_Buffer[1] >> 3;
	if(i2c_Recv_Buffer[0] >= 128)
		b = (b - 8192) / 16;
	else
		b = b / 16;
	digital_board_temp = b;

	i_sprintf_ret = snprintf(report_buff, 100, "%d_%d_%u_%llu\n", analog_board_temp, digital_board_temp, i_neutron_total, local_time);
	XUartPs_Send(&Uart_PS, (u8 *)report_buff, i_sprintf_ret);

	return 0;
}
//////////////////////////// Report SOH Function ////////////////////////////////
#endif

//////////////////////////// Clear Processed Data Buffers ////////////////////////////////
void ClearBuffers() {
	Xil_Out32(XPAR_AXI_GPIO_9_BASEADDR,1);
	usleep(1);
	Xil_Out32(XPAR_AXI_GPIO_9_BASEADDR,0);
}
//////////////////////////// Clear Processed Data Buffers ////////////////////////////////

//////////////////////////// get_data ////////////////////////////////
#ifndef BREAKUP_MAIN
int get_data(XUartPs * Uart_PS, char * EVT_filename0, char * CNT_filename0, char * EVT_filename1, char * CNT_filename1, int i_neutron_total, char * RecvBuffer, XTime local_time_start, XTime local_time)
#else
int get_data(XUartPs Uart_PS, char * EVT_filename0, char * CNT_filename0, char * EVT_filename1, char * CNT_filename1, char * RecvBuffer)
#endif
{
	int valid_data = 0; 	//BRAM buffer size
	int buff_num = 0;	//keep track of which buffer we are writing
	int array_index = 0;
	int dram_addr;
	int dram_base = 0xa000000;
	int dram_ceiling = 0xA004000;
	int ipollReturn = 0;	//keep track of user input
	//2DH variables
	int i_xnumbins = 260;
	int i_ynumbins = 30;
	//buffers are 4096 ints long (512 events total)
	unsigned int * data_array;
	unsigned int * data_array_holder;
	data_array = (unsigned int *)malloc(sizeof(unsigned int)*DATA_BUFFER_SIZE*4);
	memset(data_array, '0', DATA_BUFFER_SIZE * sizeof(unsigned int)); //zero out the array
	unsigned short twoDH_pmt1[i_xnumbins][i_ynumbins];
	unsigned short twoDH_pmt2[i_xnumbins][i_ynumbins];
	unsigned short twoDH_pmt3[i_xnumbins][i_ynumbins];
	unsigned short twoDH_pmt4[i_xnumbins][i_ynumbins];
	FIL data_file;
	FIL data_file_dest;

	//timing
	XTime local_time_current;
	local_time_current = 0;

	XUartPs_SetOptions(&Uart_PS,XUARTPS_OPTION_RESET_RX);

	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000); 		// DMA Transfer Step 1
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);			// DMA Transfer Step 2
	sleep(1);
	ClearBuffers();

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
				process_data(data_array_holder, twoDH_pmt1, twoDH_pmt2, twoDH_pmt3, twoDH_pmt4);
				ClearBuffers();
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
				process_data(data_array_holder, twoDH_pmt1, twoDH_pmt2, twoDH_pmt3, twoDH_pmt4);
				ClearBuffers();
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
				process_data(data_array_holder, twoDH_pmt1, twoDH_pmt2, twoDH_pmt3, twoDH_pmt4);
				ClearBuffers();
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
				process_data(data_array_holder, &(twoDH_pmt1[0][0]), &(twoDH_pmt2[0][0]), &(twoDH_pmt3[0][0]), &(twoDH_pmt4[0][0]));
				ClearBuffers();
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
		CheckForSOH(Uart_PS);
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
	return(GetNeuronTotal());  // is there a reasons to return neuron total will if changed it will be with PutNeuronTotal in lunah_utils.c
}

//////////////////////////// get_data ////////////////////////////////
