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

int SetFileName( int ID_number, int run_number )
{
	int status = CMD_SUCCESS;
	int bytes_written = 0;

	bytes_written = snprintf(current_filename_EVT, 100, "0:/%d_%d_evt.bin", ID_number, run_number);	//create the event-by-event filename
	if(bytes_written == 0)
		status = CMD_FAILURE;
	bytes_written = snprintf(current_filename_CPS, 100, "0:/%d_%d_cps.bin", ID_number, run_number);	//create the string to tell CDH
	if(bytes_written == 0)
		status = CMD_FAILURE;
	bytes_written = snprintf(current_filename_2DH, 100, "0:/%d_%d_2dh.bin", ID_number, run_number);	//create the string to tell CDH
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

int WriteHeaderFile( unsigned long long int real_time, float modu_temp)
{
	int status = CMD_SUCCESS;

	//create the "first event" which will be used as a header in the CPS data file
	//we don't need to actually write an "event" into the data file, we just need to know
	// what is in the file so that when we're writing it out, it gets read appropriately

	//create the "first event" used for the EVTS file

	return status;
}
