/*
 * RecordFiles.c
 *
 *  Created on: Aug 23, 2019
 *      Author: gstoddard
 */

#include "RecordFiles.h"

//static variables (file scope globals)
static char m_FILES[MAX_FILENAME_SIZE] = "FILES.bin";

static int sd_total_folders;
static int sd_total_files;

//Variables for LS function
static FILINFO sd_fno;			//this is static because we call this function recursively
static TCHAR sd_LFName[_MAX_LFN + 1];	//we keep sd_fno static across recursive calls, keep this, too

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
	tx_records[iter].file_deleted = 0;
	tx_records[iter].file_flags = 0;

	//double check that the other records we are writing are zeroed out //skip the first record, which is this file
	for(iter = 1; iter < NUM_FILES_PER_FOLDER; iter++)
	{
		strcpy(tx_records[iter].filename,record_filename);
		tx_records[iter].total_file_size = 0;
		tx_records[iter].bytes_transferred = 0;
		tx_records[iter].file_deleted = 0;
		tx_records[iter].file_flags = 0;
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
void sd_updateFileRecords( char * filename, int transfer_byteVal )
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
				files_on_disk[iter].file_deleted = 0;
				files_on_disk[iter].file_flags = 0;
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

/*
 * Delete a file record from the TX_bytes file. This should be called after the f_unlink function succeeds otherwise
 *  the file could be left on the SD card, but we would lose track of it.
 *
 * @param	(char *) pointer to the filename of the file we want to remove the record for
 *
 * @return	N/A
 */
void sd_deleteFileRecord( char * filename )
{


	return;
}

/*
 * Total folder handling and variable access functions
 */
int sd_totalFoldersIncrement( void )
{
	return ++sd_total_folders;
}

int sd_totalFoldersDecrement( void )
{
	return --sd_total_folders;
}

int sd_getTotalFolders( void )
{
	return sd_total_folders;
}

void sd_setTotalFolders( int num_folders )
{
	sd_total_folders = num_folders;
	return;
}

/*
 * Total file handling and variable access functions
 */
int sd_totalFilesIncrement( void )
{
	return ++sd_total_files;
}

int sd_totalFilesDecrement( void )
{
	return --sd_total_files;
}

int sd_getTotalFiles( void )
{
	return sd_total_files;
}

void sd_setTotalFiles( int num_files )
{
	sd_total_files = num_files;
	return;
}

/*
 * Function to loop over the SD card directories and count up the number of files and folders.
 * There is a distinction made between
 */
int SDCountFilesOnCard( char *path )
{
	int status = 0;

	return status;
}

/*
 * Function to scan the contents of the Root directory and all folders on the Root directory.
 *
 * @param	path to the directory to be scanned
 * 			use this command with "0" or "1" to scan the entire Root directory
 * @param	(char *)pointer to the buffer to use for the DIR packets
 * 				Since this function is called recursively, we must pass this through so the same
 * 				buffer is used, rather than multiple buffers.
 */
int SDScanFilesOnCard( char * path, unsigned char *packet_buffer )
{
	int status = 0;

	/*
	 * This function is taken from the elm-chan website: http://elm-chan.org/fsw/ff/doc/readdir.html
	 * I have changed the function from the website so that we can pull the folder and file names,
	 *  and the file sizes and print them to a char buffer that can be sent as a packet.
	*/
	FRESULT res;
	DIR dir;
	UINT i;
	char *fn;
	sd_fno.lfname = sd_LFName;
	sd_fno.lfsize = sizeof(sd_LFName);

	res = f_opendir(&dir, path);							/* Open the directory */
	if (res == FR_OK) {
		for (;;)
		{
			res = f_readdir(&dir, &sd_fno);					/* Read a directory item */
			if (res != FR_OK || sd_fno.fname[0] == 0)break;	/* Break on error or end of dir */
			if (sd_fno.fname[0] == '.') continue;			/* Ignore the dot entry */
			//check what type of name we should use
			fn = *sd_fno.lfname ? sd_fno.lfname : sd_fno.fname;
			if (sd_fno.fattrib & AM_DIR)
			{												/* It is a directory */
				if(!strcmp(fn, "System Volume Information"))break;	//if we match a system folder, don't print it
				i = strlen(path);
				sprintf(&path[i], "/%s", fn);
				xil_printf("%s\n", path);					/* Print the directory name */
				res = SDScanFilesOnCard(path, packet_buffer);				/* Enter the directory */
				if (res != FR_OK)
					break;
				path[i] = 0;
			}
			else											/* It is a file. */
				xil_printf("%s/%s\n", path, fn);
		}
		f_closedir(&dir);
	}

	return status;
}
