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

static FIL m_EVT_file;
static FIL m_CPS_file;
static FIL m_2DH_file;

//Data buffer which can hold 4096*4 integers, each buffer holds 512 8-integer events, x4 for four buffers
static unsigned int data_array[DATA_BUFFER_SIZE * 4];

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

/* Creates the data acquisition files for the run requested by the DAQ command.
 * Uses the filenames which are created from the ID number sent with the DAQ
 *  command to open and write the header into the files.
 * The files are left open by this function intentionally so that DAQ doesn't
 *  have to spend the time opening them.
 *
 * @param	None
 *
 * @return	Success/failure based on how we finished the run:
 * 			BREAK (0)	 = failure
 * 			Time Out (1) = success
 * 			END (2)		 = success
 */
int CreateDAQFiles( void )
{
	char * file_to_open = NULL;
	int iter = 0;
	int status = CMD_SUCCESS;
	uint NumBytesWr;
	FIL *DAQ_file = NULL;
	FRESULT ffs_res;
	struct DATA_FILE_HEADER_TYPE file_header_to_write;

	file_header_to_write.configBuff = *GetConfigBuffer();		//dereference to copy the struct
	//TODO: check the return was not NULL
	file_header_to_write.IDNum = daq_run_id_number;
	file_header_to_write.RunNum = daq_run_run_number;
	file_header_to_write.SetNum = daq_run_set_number;
	file_header_to_write.TempCorrectionSetNum = 1;		//will have to get this from somewhere
	file_header_to_write.EventIDFF = 0xFF;

//	TODO: do we need to check to see if any of the FILs are NULL?
	//they should be automatically created when the program starts, but...good practice to check them

	//just need to open EVT, CPS, 2DH files for DAQ, if WAV, make a switch
	for(iter = 0; iter < 3; iter++)
	{
		switch(iter)
		{
		case 0:
			file_to_open = current_filename_EVT;
			file_header_to_write.FileTypeAPID = 0x77;
			DAQ_file = &m_EVT_file;
			break;
		case 1:
			file_to_open = current_filename_CPS;
			file_header_to_write.FileTypeAPID = 0x55;
			DAQ_file = &m_CPS_file;
			break;
		case 2:
			file_to_open = current_filename_2DH;
			file_header_to_write.FileTypeAPID = 0x88;
			DAQ_file = &m_2DH_file;
			break;
		default:
			status = CMD_FAILURE;
			break;
		}

		ffs_res = f_open(DAQ_file, file_to_open, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
		if(ffs_res == FR_OK)
		{
			ffs_res = f_lseek(DAQ_file, 0);
			if(ffs_res == FR_OK)
			{
				ffs_res = f_write(DAQ_file, &file_header_to_write, sizeof(file_header_to_write), &NumBytesWr);
				if(ffs_res == FR_OK && NumBytesWr == sizeof(file_header_to_write))
				{
					ffs_res = f_sync(DAQ_file);
					if(ffs_res == FR_OK)
						status = CMD_SUCCESS;
					else
						status = CMD_FAILURE;
				}
				else
					status = CMD_FAILURE;
			}
			else
				status = CMD_FAILURE;
		}
		else
			status = CMD_FAILURE;
	}

	return status;
}


FIL *GetEVTFilePointer( void )
{
	return &m_EVT_file;
}

FIL *GetCPSFilePointer( void )
{
	return &m_CPS_file;
}

FIL *Get2DHFilePointer( void )
{
	return &m_2DH_file;
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
 * @param	(XIicPs *) Pointer to Iic instance (for read temp while in DAQ)
 *
 * @param	(XUartPs) UART instance for reporting SOH
 *
 * @param	(char *) Pointer to the receive buffer for getting user input
 *
 * @param	(integer) Time out value indicating when to break out of DAQ in minutes
 * 			Ex. 1 = loop for 1 minute
 *
 *
 * @return	Success/failure based on how we finished the run:
 * 			BREAK (0)	 = failure
 * 			Time Out (1) = success
 * 			END (2)		 = success
 */
int DataAcquisition( XIicPs * Iic, XUartPs Uart_PS, char * RecvBuffer, int time_out )
{
	//initialize variables
	int done = 0;				//local status variable for keeping track of progress within loops
	int status = CMD_SUCCESS;	//local status variable
	int poll_val = 0;			//local polling status variable
	int valid_data = 0;			//goes high/low if there is valid data within the FPGA buffers
	int buff_num = 0;			//keep track of which buffer we are writing
	int m_buffers_written = 0;	//keep track of how many buffers are written, but not synced
	int array_index = 0;		//the index of our array which will hold data
	int dram_addr;				//the address in the DRAM we are reading from
	int dram_base = 0xA000000;	//where the buffer starts	//167,772,160
	int dram_ceiling = 0xA004000;	//where it ends			//167,788,544
	int m_run_time = time_out * 60;	//multiply minutes by 60 to get seconds
	int m_write_header = 1;		//write a file header the first time we use a file
	XTime m_run_start;			//timing variable
	XTime m_run_current_time;	//timing variable
	XTime_GetTime(&m_run_start);//record the "start" time to base a time out on
	unsigned int m_first_event_FPGA_time = 0;
	unsigned long long m_spacecraft_real_time = 0;
	unsigned char m_write_times_into_header[SIZEOF_HEADER_TIMES] = "";
	unsigned int bytes_written = 0;
	FRESULT f_res = FR_OK;
	GENERAL_EVENT_TYPE * evts_array = NULL;

	//getting raw data with these variables
	unsigned int numBytesWritten = 0;
	FIL rawData;
	f_res = f_open(&rawData, "rawDat01.bin", FA_WRITE|FA_READ|FA_OPEN_ALWAYS);
	if(f_res != FR_OK)
		xil_printf("1 open file fail DAQ\n");
	f_res = f_lseek(&rawData, file_size(&rawData));
	if(f_res != FR_OK)
		xil_printf("2 lseek fail DAQ\n");

	ResetEVTsBuffer();
	ResetEVTsIterator();
	ClearBRAMBuffers();
	while(done != 1)
	{
		//check the FPGA to see if there is valid data in the buffers
		//bit set high (1) when there is at least one valid (full) buffer of data in the FPGA
		valid_data = Xil_In32 (XPAR_AXI_GPIO_11_BASEADDR);
		if(valid_data == 1)
		{
			//init/start MUX to transfer data between integrator modules and the DMA
			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 1);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x48, 0xa000000);
			Xil_Out32 (XPAR_AXI_DMA_0_BASEADDR + 0x58 , 65536);
			usleep(54);
			//TODO: need to check a shared variable within the interrupt handler and this function
			// to see if the transfer is completed
			//This check would replace the sleep statement.

			Xil_Out32 (XPAR_AXI_GPIO_15_BASEADDR, 0);

			ClearBRAMBuffers();

			Xil_DCacheInvalidateRange(0xa0000000, 65536);

			array_index = 0;
			dram_addr = dram_base;
			switch(buff_num)
			{
			case 0:
				while(dram_addr < dram_ceiling) //Does this need to be non-inclusive? Can we include the dram_ceiling? //TRYING THIS 2/26/19 GJS
				{
					data_array[array_index + DATA_BUFFER_SIZE * buff_num] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				status = ProcessData( &data_array[DATA_BUFFER_SIZE * buff_num] );
				buff_num++;
				break;
			case 1:
				while(dram_addr < dram_ceiling)
				{
					data_array[array_index + DATA_BUFFER_SIZE * buff_num] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				status = ProcessData( &data_array[DATA_BUFFER_SIZE * buff_num] );
				buff_num++;
				break;
			case 2:
				while(dram_addr < dram_ceiling)
				{
					data_array[array_index + DATA_BUFFER_SIZE * buff_num] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				status = ProcessData( &data_array[DATA_BUFFER_SIZE * buff_num] );
				buff_num++;
				break;
			case 3:
				while(dram_addr < dram_ceiling)
				{
					data_array[array_index + DATA_BUFFER_SIZE * buff_num] = Xil_In32(dram_addr);
					dram_addr += 4;
					array_index++;
				}
				f_res = f_lseek(&rawData, file_size(&rawData));
				f_res = f_write(&rawData, data_array, sizeof(int) * 4096 * 4, &numBytesWritten);	//TEST LINE
				if(f_res != FR_OK)
					xil_printf("6 write fail DAQ\n");
				status = ProcessData( &data_array[DATA_BUFFER_SIZE * buff_num] );
				buff_num = 0;

				//If this is the first time that we have used a file, write in the header
				if(m_write_header == 1)
				{
					//get the first event and the real time
					m_first_event_FPGA_time = GetFirstEventTime();
					m_spacecraft_real_time = GetRealTimeParam();
					memcpy(&(m_write_times_into_header[0]), &m_spacecraft_real_time, sizeof(m_spacecraft_real_time));
					m_write_times_into_header[8] = 0xFF;
					memcpy(&(m_write_times_into_header[9]), &m_first_event_FPGA_time, sizeof(m_first_event_FPGA_time));
					m_write_times_into_header[13] = 0xFF;
					//write into the file
					f_res = f_write(&m_EVT_file, m_write_times_into_header, SIZEOF_HEADER_TIMES, &bytes_written);
					if(f_res != FR_OK || bytes_written != SIZEOF_HEADER_TIMES)
					{
						//TODO: handle error checking the write
						xil_printf("10 error writing DAQ\n");
					}
					m_write_header = 0;	//turn off header writing
				}
				evts_array = GetEVTsBufferAddress();
				//TODO: check that the evts_array address is not NULL
				f_res = f_write(&m_EVT_file, evts_array, EVTS_DATA_BUFF_SIZE, &bytes_written); //write the entire events buffer
				if(f_res != FR_OK || bytes_written != EVTS_DATA_BUFF_SIZE)
				{
					//TODO: handle error checking the write here
					xil_printf("7 error writing DAQ\n");
				}
				m_buffers_written++;
				if(f_res == FR_OK && m_buffers_written == 4)
				{
					f_res = f_sync(&m_EVT_file);
					if(f_res != FR_OK)
					{
						//TODO: error check
						xil_printf("8 error syncing DAQ\n");
					}
					f_res = f_sync(&rawData);
					if(f_res != FR_OK)
						xil_printf("9 sync fail DAQ\n");
					m_buffers_written = 0;	//reset
				}
				//reset the EVTs array
				ResetEVTsBuffer();
				ResetEVTsIterator();
				break;
			default:
				//TODO: fill in the default behavior when we don't get the right buff_num
				//maybe try and figure out what buffer this should be?
				// could be worth trying to figure out what went wrong with buff_num and fix that
				//otherwise maybe just throw out everything and start over (zero out most stuff)
				// this could maybe get us back to a "good" state, or at least a known one?
				break;
			}
			valid_data = 0;	//reset

		}//END OF IF VALID DATA

		//check to see if it is time to report SOH information, 1 Hz
		CheckForSOH(Iic, Uart_PS);

		//check timeout condition //calculate run time?
		//TODO: check the time it takes to run this
		XTime_GetTime(&m_run_current_time);
		if(((m_run_current_time - m_run_start)/COUNTS_PER_SECOND) >= m_run_time)
		{
			status = DAQ_TIME_OUT;
			done = 1;
		}

		//TODO: TESTING 3/15/2019
		//to have this break after one go-around, just comment "poll_val = ReadCommandType();
		// and set poll_val = BREAK_CMD; so we jump out

		//check for user input
		if(buff_num == 1)			//if we have read one buffer of data, jump out for testing purposes
			poll_val = BREAK_CMD;
		else
			poll_val = ReadCommandType(RecvBuffer, &Uart_PS);

		switch(poll_val)
		{
		case -1:
			//this is bad input or an error in input
			//no real need for a case if we aren't handling it
			//just leave this to default
			break;
		case READ_TMP_CMD:
			status = report_SOH(Iic, GetLocalTime(), GetNeutronTotal(), Uart_PS, READ_TMP_CMD);
			if(status == CMD_FAILURE)
				reportFailure(Uart_PS);
			break;
		case BREAK_CMD:
			//write in footer to data files

			f_close(&m_EVT_file);
			f_close(&m_CPS_file);
			f_close(&m_2DH_file);
			f_close(&rawData);
			status = DAQ_BREAK;
			done = 1;
			break;
		case END_CMD:
			//write in footer to data files

			f_close(&m_EVT_file);
			f_close(&m_CPS_file);
			f_close(&m_2DH_file);
			f_close(&rawData);
			status = DAQ_END;
			done = 1;
			break;
		default:
			break;
		}
	}//END OF WHILE DONE != 1

	//here is where we should transfer the CPS, 2DH files?

	//cleanup operations
	if(poll_val != BREAK_CMD && poll_val != END_CMD) // && status != DAQ_TIME_OUT) //currently need to handle closing files when breaking due to timeout
	{
		f_close(&m_EVT_file);
		f_close(&m_CPS_file);
		f_close(&m_2DH_file);
		f_close(&rawData);
	}

	return status;
}
