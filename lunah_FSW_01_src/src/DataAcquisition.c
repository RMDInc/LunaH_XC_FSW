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
static unsigned char daq_run_id_number = 0;
static unsigned char daq_run_run_number = 0;
static unsigned char daq_run_set_number = 0;

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

	//create the file names, writing them to our file scope variables
	bytes_written = snprintf(current_filename_EVT, 100, "0:/evt_I%06d_R%06d_S%06d.bin", ID_number, run_number, set_number);	//create the event-by-event filename
	if(bytes_written == 0)
		status = CMD_FAILURE;
	bytes_written = snprintf(current_filename_CPS, 100, "0:/cps_I%06d_R%06d_S%06d.bin", ID_number, run_number, set_number);	//create the string to tell CDH
	if(bytes_written == 0)
		status = CMD_FAILURE;
	bytes_written = snprintf(current_filename_2DH, 100, "0:/2dh_I%06d_R%06d_S%06d.bin", ID_number, run_number, set_number);	//create the string to tell CDH
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
	FIL daq_file;
	FRESULT ffs_res;	//FAT file system return type

	//take the static file names above and open the files with appropriate permissions for DAQ
	//just need to open EVT, CPS, 2DH files for DAQ, if WAV, can set a bypass
	for(iter = 0; iter < 3; iter++)
	{
		//loop over the three files to create
		switch(iter)
		{
		case 0:	//choose the event-by-event file
			file_to_open = current_filename_EVT;
			break;
		case 1:	//choose the counts-per-second file
			file_to_open = current_filename_CPS;
			break;
		case 2:	//choose the two-D histogram file
			file_to_open = current_filename_2DH;
			break;
		default:
			status = CMD_FAILURE;
			break;
		}
		//open and close each file to create it
		ffs_res = f_open(&daq_file, file_to_open, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);	//open the file, if it exists; if not a new file will be created
		if(ffs_res == FR_OK)
			ffs_res = f_lseek(&daq_file, 0);

		//call write data file header here so we can

		if(ffs_res == FR_OK)
			ffs_res = f_close(&daq_file);
	}

	return status;
}

int WriteDataFileHeader( unsigned long long int real_time, float modu_temp)
{
	int status = CMD_SUCCESS;

	//create the "first event" which will be used as a header in the CPS data file
	//we don't need to actually write an "event" into the data file, we just need to know
	// what is in the file so that when we're writing it out, it gets read appropriately

	//create the "first event" used for the EVTS file

	return status;
}

//Clears the BRAM buffers
// I need to refresh myself as to why this is important
// All that I remember is that it's important to do before each DRAM transfer
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
int DataAcquisition( void )
{
	//initialize variables
	int done = 0;				//local status variable for keeping track of progress within loops
	int status = CMD_SUCCESS;	//local status variable
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
	data_array = calloc(DATA_BUFFER_SIZE * 4, sizeof(unsigned int));
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
				//fetch the data from the DRAM
				while(dram_addr <= dram_ceiling)
				{
					data_array[array_index] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				//we have collected all the data, process it and then get back to check for more data
				//status = ProcessData();
				//handle buffer number within each case
				buff_num++;
				break;
			case 1:
				//fetch the data from the DRAM

				while(dram_addr <= dram_ceiling)
				{
					data_array[array_index] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				//we have collected all the data, process it and then get back to check for more data
				//status = ProcessData();
				//handle buffer number within each case
				buff_num++;
				break;
			case 2:
				//fetch the data from the DRAM
				while(dram_addr <= dram_ceiling)
				{
					data_array[array_index] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				//we have collected all the data, process it and then get back to check for more data
				//status = ProcessData();
				//handle buffer number within each case
				buff_num++;
				break;
			case 3:
				//fetch the data from the DRAM
				while(dram_addr <= dram_ceiling)
				{
					data_array[array_index] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				//we have collected all the data, process it and then get back to check for more data
				//status = ProcessData();
				//handle buffer number within each case
				buff_num = 0;

				//four buffers have been processed, write the data to file


				break;
			default:
				break;
			}
			//clear the buffers after reading through the data
			ClearBRAMBuffers();
			valid_data = 0;	//reset
		}
	}

	//cleanup operations
	free(data_array);

	return status;
}
