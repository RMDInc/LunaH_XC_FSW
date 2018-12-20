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

int SetFileName( int ID_number, int run_number, int set_number )
{
	int status = CMD_SUCCESS;
	int bytes_written = 0;

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
		if(ffs_res == FR_OK)
			ffs_res = f_close(&daq_file);
	}

	return status;
}

int WriteHeaderFile( unsigned long long int real_time, float modu_temp)
{
	int status = CMD_SUCCESS;

	//create the "first event" which will be used as a header in the CPS data file
	//we don't need to actually write an "event" into the data file, we just need to know
	// what is in the file so that when we're writing it out, it gets read appropriately

	//create the "first event" used for the EVTS file

	return status;
}
