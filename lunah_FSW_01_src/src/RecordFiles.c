/*
 * RecordFiles.c
 *
 *  Created on: Aug 23, 2019
 *      Author: gstoddard
 */

#include "RecordFiles.h"

//static variables (file scope globals)
static char m_FILES[MAX_FILENAME_SIZE] = "FILES.bin";

void sd_createTXBytesFile( void )
{
	TX_BYTES_FILE_TYPE tx_records[NUM_FILES_PER_FOLDER] = {};	//6 data product files plus itself
	FRESULT ffs_res;
	FIL txbytesFIL;
	//Initialize the FILINFO struct for LFN
	FILINFO txbytesFILINFO;
	TCHAR LFName[256];
	txbytesFILINFO.lfname = LFName;
	txbytesFILINFO.lfsize = sizeof(LFName);
	char record_filename[MAX_FILENAME_SIZE] = "";
	int iter = 0;
	unsigned int bytesWr = 0;

	//write in the record for this file first
	strcpy(tx_records[iter].filename, m_FILES);
	tx_records[iter].total_file_size = sizeof(TX_BYTES_FILE_TYPE) * NUM_FILES_PER_FOLDER;
	tx_records[iter].bytes_transferred = 0;

	//double check that the other records we are writing are zeroed out //skip the first record, which is this file
	for(iter = 1; iter < NUM_FILES_PER_FOLDER; iter++)
	{
		strcpy(tx_records[iter].filename,record_filename);
		tx_records[iter].total_file_size = 0;
		tx_records[iter].bytes_transferred = 0;
	}

	//open the tx_bytes.txt file
	ffs_res = f_open(&txbytesFIL, m_FILES, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	if(ffs_res != FR_OK)
		xil_printf("error open FILES.bin\n");

	ffs_res = f_write(&txbytesFIL, &tx_records, sizeof(TX_BYTES_FILE_TYPE) * NUM_FILES_PER_FOLDER, &bytesWr);
	if(ffs_res != FR_OK || bytesWr != sizeof(TX_BYTES_FILE_TYPE) * NUM_FILES_PER_FOLDER)
		xil_printf("error writing tx bytes file, bytes written = %d\n", bytesWr);
	else
	{
		ffs_res = f_stat(tx_records[0].filename, &txbytesFILINFO);
		xil_printf("Wrote %d bytes to %s\n", bytesWr, txbytesFILINFO.fname);
	}

	f_close(&txbytesFIL);
	return;
}

//function to
void sd_updateTXfile( char * filename, int transfer_byteVal )
{
	//variables
	FRESULT ffs_res;
	FIL txbytesFIL;
	FILINFO fno;
	//Initialize the FILINFO struct for LFN
	TCHAR LFName[256];
	fno.lfname = LFName;
	fno.lfsize = sizeof(LFName);
	int iter = 0;
	unsigned int bytesWr = 0;
	unsigned int bytesRead = 0;
	char no_filename[MAX_FILENAME_SIZE] = "";
	TX_BYTES_FILE_TYPE files_on_disk[NUM_FILES_PER_FOLDER] = {};

	ffs_res = f_open(&txbytesFIL, m_FILES, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	if(ffs_res != FR_OK)
		xil_printf("error open FILES.bin\n");
	ffs_res = f_stat(filename, &fno);
	if(ffs_res != FR_OK)
		xil_printf("error f_stat\n");

	//read in the structs from the TX file
	ffs_res = f_lseek(&txbytesFIL, 0);
	ffs_res = f_read(&txbytesFIL, &files_on_disk, sizeof(TX_BYTES_FILE_TYPE) * NUM_FILES_PER_FOLDER, &bytesRead);
	if(ffs_res != FR_OK)
	{
		xil_printf("error f_read tx bytes\n");
	}
	else if(bytesRead != sizeof(TX_BYTES_FILE_TYPE) * NUM_FILES_PER_FOLDER)
	{
		//either file is new or we have only written a few file names to it
		xil_printf("error f_read too few bytes\nbytesRead = %d, size should be = %d\n", bytesRead, sizeof(TX_BYTES_FILE_TYPE) * NUM_FILES_PER_FOLDER);
	}
	else
	{
		//loop trying to match the names
		iter = 0;
		while(iter < NUM_FILES_PER_FOLDER)
		{
			//compare each record name with the file that we want to update
			if(strcmp(files_on_disk[iter].filename, fno.fname) == 0)
			{
				//we found a match //compare the total size
				if(files_on_disk[iter].total_file_size != fno.fsize)
					files_on_disk[iter].total_file_size = fno.fsize;
				//check if we should update the transfer bytes
				if(files_on_disk[iter].bytes_transferred != transfer_byteVal)
				{
					if(files_on_disk[iter].bytes_transferred != NO_TX_CHANGE && transfer_byteVal <= fno.fsize)
						files_on_disk[iter].bytes_transferred = transfer_byteVal;
				}
				break;
			}
			else if(strcmp(files_on_disk[iter].filename, no_filename) == 0)
			{
				//we found no matches before we found an empty record, write the file and size into this record
				strcpy(files_on_disk[iter].filename,fno.fname);
				files_on_disk[iter].total_file_size = fno.fsize;
				files_on_disk[iter].bytes_transferred = 0;
				break;
			}
			else
				iter++;
		}
	}

	//write the updates back into the file by rewriting the file, we have copied out the values
	ffs_res = f_lseek(&txbytesFIL, 0);	//seek to the beginning, we just want to overwrite all the previous records
	ffs_res = f_write(&txbytesFIL, (char *)&files_on_disk, sizeof(TX_BYTES_FILE_TYPE) * NUM_FILES_PER_FOLDER, &bytesWr);
	if(ffs_res != FR_OK || bytesWr != sizeof(TX_BYTES_FILE_TYPE) * NUM_FILES_PER_FOLDER)
		xil_printf("error writing updated structs into file\n");

	f_close(&txbytesFIL);
	return;
}
