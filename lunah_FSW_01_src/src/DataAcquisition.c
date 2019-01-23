/*
 * DataAcquisition.c
 *
 *  Created on: Dec 4, 2018
 *      Author: IRDLab
 */

#include "DataAcquisition.h"

//FILE SCOPE VARIABLES
static char current_filename_EVT[100] = "";
static char current_filename_CPS[100] = "";
static char current_filename_2DH[100] = "";
static char current_filename_WAV[100] = "";
static unsigned int daq_run_id_number = 0;
static unsigned int daq_run_run_number = 0;
static unsigned int daq_run_set_number = 0;

//We only want to use this here for now, so hide it from the user
//This is a struct featuring the information from the config buffer
// plus a few extra pieces that need to go into headers.
struct DATA_FILE_HEADER_TYPE{
	CONFIG_STRUCT_TYPE configBuff;
	unsigned int IDNum;
	unsigned int RunNum;
	unsigned int SetNum;
	unsigned char FileTypeAPID;
	unsigned char TempCorrectionSetNum;
	unsigned char EventIDFF;
};

/*
 * Getter function for the size of the filenames which are assembled by the system
 * This function doesn't need a file type because the filenames were designed to
 * have the same length.
 *
 * @param	None
 *
 * @return	The number of bytes in the length of the filename string
 */
int GetFileNameSize( void )
{
	return (int)strlen(current_filename_EVT);
}

/* Getter function for the current data acquisition filename string
 *
 * Each function and file name will have to be assembled from this string
 * which will be composed of the following parts:
 *
 * IDNum_RunNum_TYPE.bin
 *
 * ID Number 	= user input value which is the first unique value
 * Run Number 	= Mini-NS tracked value which counts how many runs have been made since the last POR
 * TYPE			= EVTS	-> event-by-event data product
 * 				= CPS 	-> counts-per-second data product
 * 				= WAV	-> waveform data product
 * 				= 2DH	-> two-dimensional histogram data product
 *
 * @param	None
 *
 * @return	Pointer to the buffer holding the filename.
 *
 */
char * GetFileName( int file_type )
{
	char * current_filename;

	switch(file_type)
	{
	case DATA_TYPE_EVTS:
		current_filename = current_filename_EVT;
		break;
	case DATA_TYPE_WAV:
		current_filename = current_filename_WAV;
		break;
	case DATA_TYPE_2DH:
		current_filename = current_filename_2DH;
		break;
	case DATA_TYPE_CPS:
		current_filename = current_filename_CPS;
		break;
	default:
		current_filename = NULL;
		break;
	}

	return current_filename;
}

/*
 * Getter function for the current DAQ run ID number.
 * This value is provided by the user and stored.
 *
 * @param	None
 *
 * @return	The ID number provided to the MNS_DAQ command from the most recent/current run
 */
unsigned int GetDAQRunIDNumber( void )
{
	return daq_run_id_number;
}

/*
 * Getter function for the current DAQ run RUN number.
 * This value is calculated by the system and stored for internal use and in creating
 *  unique filenames for the data products being stored to the SD card.
 * This value is zeroed out each time the Mini-NS power cycles.
 * This value is incremented each time the Mini-NS begins a new DAQ run.
 *
 * @param	None
 *
 * @return	The number of DAQ runs since the system power cycled last, the RUN number
 */
unsigned int GetDAQRunRUNNumber( void )
{
	return daq_run_run_number;
}

/*
 * Getter function for the current DAQ run SET number.
 * As the Mini-NS is collecting data, the EVTS data product files will become quite large. To
 *  mitigate the problem of having to downlink very large files with limited bandwidth, the
 *  system will close a file which exceeds ~1MB in size. It will then open a file with the same
 *  filename, except the SET number will be incremented by 1. It will continue recording data in
 *  that file for the run.
 * This value is zeroed out for each DAQ run.
 * This value is incremented each time the Mini-NS closes a data product file to start a new one.
 *
 * @param	None
 *
 * @return	The number of DAQ runs since the system power cycled last, the RUN number
 */
unsigned int GetDAQRunSETNumber( void )
{
	return daq_run_set_number;
}

int SetFileName( int ID_number, int run_number, int set_number )
{
	int status = CMD_SUCCESS;
	int bytes_written = 0;

	//save the values so we can access them later, we can put them in the file headers
	daq_run_id_number = ID_number;
	daq_run_run_number = run_number;
	daq_run_set_number = set_number;

	bytes_written = snprintf(current_filename_EVT, 100, "0:/evt_I%06d_R%06d_S%06d.bin", ID_number, run_number, set_number);
	if(bytes_written == 0)
		status = CMD_FAILURE;
	bytes_written = snprintf(current_filename_CPS, 100, "0:/cps_I%06d_R%06d_S%06d.bin", ID_number, run_number, set_number);
	if(bytes_written == 0)
		status = CMD_FAILURE;
	bytes_written = snprintf(current_filename_2DH, 100, "0:/2dh_I%06d_R%06d_S%06d.bin", ID_number, run_number, set_number);
	if(bytes_written == 0)
		status = CMD_FAILURE;

	return status;
}

int DoesFileExist( void )
{
	int status = CMD_SUCCESS;
	FILINFO fno;		//file info structure
	FRESULT ffs_res;	//FAT file system return type

	//check the SD card for the existence of the current filename
	ffs_res = f_stat(current_filename_EVT, &fno);
	if(ffs_res == FR_OK)
		ffs_res = f_stat(current_filename_CPS, &fno);
	if(ffs_res == FR_OK)
		ffs_res = f_stat(current_filename_2DH, &fno);
	else
		status = CMD_FAILURE;

	return status;
}

int CreateDAQFiles( void )
{
	char * file_to_open;
	int iter = 0;
	int status = CMD_SUCCESS;
	uint NumBytesWr;
	FIL daq_file;
	FRESULT ffs_res;
	struct DATA_FILE_HEADER_TYPE file_header_to_write;

	file_header_to_write.configBuff = *GetConfigBuffer();		//dereference to copy the struct
	file_header_to_write.IDNum = daq_run_id_number;
	file_header_to_write.RunNum = daq_run_run_number;
	file_header_to_write.SetNum = daq_run_set_number;
	file_header_to_write.TempCorrectionSetNum = 1;		//will have to get this from somewhere
	file_header_to_write.EventIDFF = 0xFF;

	//just need to open EVT, CPS, 2DH files for DAQ, if WAV, make a switch
	for(iter = 0; iter < 3; iter++)
	{
		switch(iter)
		{
		case 0:
			file_to_open = current_filename_EVT;
			file_header_to_write.FileTypeAPID = 0x77;
			break;
		case 1:
			file_to_open = current_filename_CPS;
			file_header_to_write.FileTypeAPID = 0x55;
			break;
		case 2:
			file_to_open = current_filename_2DH;
			file_header_to_write.FileTypeAPID = 0x88;
			break;
		default:
			status = CMD_FAILURE;
			break;
		}

		ffs_res = f_open(&daq_file, file_to_open, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
		if(ffs_res == FR_OK)
			ffs_res = f_lseek(&daq_file, 0);
		if(ffs_res == FR_OK)
			ffs_res = f_write(&daq_file, &file_header_to_write, sizeof(file_header_to_write), &NumBytesWr);
		if(ffs_res == FR_OK)
			ffs_res = f_close(&daq_file);
		if(ffs_res == FR_OK && NumBytesWr != 0)
			status = CMD_SUCCESS;
		else
			status = CMD_FAILURE;
	}

	return status;
}

int WriteRealTime( unsigned long long int real_time )
{
	int status = CMD_SUCCESS;
//	uint NumBytesWr;
//	FRESULT F_RetVal;
//	FILINFO CnfFno;
//	FIL ConfigFile;
//	int RetVal = 0;
//	int ConfigSize = sizeof(ConfigBuff);
//
//	//take the config buffer and put it into each data product file
//	// check that data product file exists
//	if( f_stat( cConfigFile, &CnfFno) )	//f_stat returns non-zero (true) if no file exists, so open/create the file
//	{
//		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_WRITE|FA_OPEN_ALWAYS);
//		if(F_RetVal == FR_OK)
//			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
//		if(F_RetVal == FR_OK)
//			F_RetVal = f_close(&ConfigFile);
//	}
//	else // If the file exists, write it
//	{
//		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_READ|FA_WRITE);	//open with read/write access
//		if(F_RetVal == FR_OK)
//			F_RetVal = f_lseek(&ConfigFile, 0);							//go to beginning of file
//		if(F_RetVal == FR_OK)
//			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);	//Write the ConfigBuff to config file
//		if(F_RetVal == FR_OK)
//			F_RetVal = f_close(&ConfigFile);							//close the file
//		}
//
//	RetVal = (int)F_RetVal;
//    return RetVal;

	return status;
}

//Clears the BRAM buffers
// I need to refresh myself as to why this is important
// All that I remember is that it's important to do before each DRAM transfer
//Resets which buffer we are reading from
// issuing this "clear" allows us to move to the next buffer to read from it
//Tells the FPGA, we are done with this buffer, read from the next one
void ClearBRAMBuffers( void )
{
	Xil_Out32(XPAR_AXI_GPIO_9_BASEADDR,1);
	usleep(1);
	Xil_Out32(XPAR_AXI_GPIO_9_BASEADDR,0);
}

/* What it's all about.
 * The main event.
 * This is where we interact with the FPGA to receive data,
 *  then process and save it. We are reporting SOH and various SUCCESS/FAILURE packets along
 *  the way.
 *
 * @param	None
 *
 * @return	Success/failure based on how we finished the run:
 * 			BREAK (0)	 = failure
 * 			Time Out (1) = success
 * 			END (2)		 = success
 */
int DataAcquisition( XIicPs * Iic, XUartPs Uart_PS, char * RecvBuffer )
{
	//initialize variables
	int done = 0;				//local status variable for keeping track of progress within loops
	int status = CMD_SUCCESS;	//local status variable
	int poll_val = 0;		//local polling status variable
	int valid_data = 0;			//goes high/low if there is valid data within the FPGA buffers
	int buff_num = 0;			//keep track of which buffer we are writing
	int array_index = 0;		//the index of our array which will hold data
	int dram_addr;				//the address in the DRAM we are reading from
	int dram_base = 0xa000000;	//where the buffer starts
	int dram_ceiling = 0xA004000;	//where it ends
	//load parameters which are needed for the run
	//	temp of the modules for the correction
	//	any structs or memory that needs to be reserved can be done here
	unsigned int * data_array;
	data_array = calloc(DATA_BUFFER_SIZE * 4, sizeof(unsigned int));	//we need a buffer which can hold 4096*4 integers, each buffer holds 512 8-integer events
	//init a DMA transfer
	//Is this necessary before we check to see if there is anything there yet?
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);	// DMA Transfer Step 1
	Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);		// DMA Transfer Step 2
	//Clear BRAM buffers
	ClearBRAMBuffers();
	//begin the valid data check loop
	while(done != 1)
	{
		//check the FPGA to see if there is valid data in the buffers
		//bit set high (1) when there is at least one valid (full) buffer of data in the FPGA
		valid_data = Xil_In32 (XPAR_AXI_GPIO_11_BASEADDR);
		if(valid_data == 1)
		{
			//init/start MUX to transfer data between integrator modules and the DMA
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);
			//DMA Transfer, step 1, 2
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			//this is a mandatory sleep which gives the system enough time to transfer the data out
			//TODO: Optimize/remove this sleep
			usleep(54);
			//I assume this turns off the MUX and stops the transfer; send when the transfer is done (we have all the data)
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0);
			//block an area of memory out for us to read from so we don't collide with anything else
			//is this necessary? I have never tried to run without this, but have messed with the settings
			// before and found no specific differences. If we are using this, need to make sure that
			// the memory range we specify is correct and that we actually need to invalidate it.
			Xil_DCacheInvalidateRange(0xa0000000, 65536);

			//prepare for looping
			array_index = 0;
			dram_addr = dram_base;
			switch(buff_num)
			{
			case 0:
				while(dram_addr <= dram_ceiling)
				{
					data_array[array_index] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				status = ProcessData( data_array );
				buff_num++;
				break;
			case 1:
				while(dram_addr <= dram_ceiling)
				{
					data_array[array_index + DATA_BUFFER_SIZE] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				//we have collected all the data, process it and then get back to check for more data
				//status = ProcessData();
				buff_num++;
				break;
			case 2:
				while(dram_addr <= dram_ceiling)
				{
					data_array[array_index + DATA_BUFFER_SIZE * 2] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				//we have collected all the data, process it and then get back to check for more data
				//status = ProcessData();
				buff_num++;
				break;
			case 3:
				while(dram_addr <= dram_ceiling)
				{
					data_array[array_index + DATA_BUFFER_SIZE * 3] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				//we have collected all the data, process it and then get back to check for more data
				//status = ProcessData();
				buff_num = 0;

				//four buffers have been processed, write the data to file
				//If this is the first time that we've written to this file
				//(will happen with first buffer or each time we change files)


				break;
			default:
				break;
			}
			//clear the buffers after reading through the data
			ClearBRAMBuffers();
			valid_data = 0;	//reset
		}//END OF IF VALID DATA

		//check to see if it is time to report SOH information, 1 Hz
		CheckForSOH(Iic, Uart_PS);

		//check for user input
		poll_val = ReadCommandType(RecvBuffer, &Uart_PS);
		switch(poll_val)
		{
		case -1:
			//this is bad input or an error in input
			//no real need for a case if we aren't handling it
			//just leave this to default
			break;
		case 16:
			break;
		case 17:
			break;
		case 18:
			break;
		default:
			break;
		}
	}

	//cleanup operations
	free(data_array);

	return status;
}
