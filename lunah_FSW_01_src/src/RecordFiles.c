/*
 * RecordFiles.c
 *
 *  Created on: Aug 23, 2019
 *      Author: gstoddard
 */

#include "RecordFiles.h"

//static variables (file scope globals)
static char m_FILES[MAX_FILENAME_SIZE] = "FILES.bin";

static int sd_count_files;
static int sd_count_folders;
static int sd_total_folders;
static int sd_total_files;

//Variables for LS function
static int iter;
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
 * Initialize variables before calling SDCountFiles() or SDScanFiles()
 *
 * @param	none
 *
 * @return	(int)status variable, success/failure
 */
void SDInitDIR( void )
{
	iter = 0;
	sd_count_folders = 0;
	sd_count_files = 0;

	return;
}

/*
 * Helper function to put together the DIR packet header, which is comprised of the following:
 *  the SD card number (0/1)
 *  the most recent Real Time listed in the configuration file
 *  the total number of folders on the SD card
 *  the total number of files on the SD card
 * The only input needed is the value for the sd card number. The primary SD card is 0, the backup
 *  card is 1. The Real Time is retrieved from the configuration file. The value for the number of files
 *  and folders can be taken from the configuration file, but it can also be checked using the
 *  SDCountFilesOnCard() function below. That will give the most up-to-date number.
 * There is no distinction made between DAQ folders/files and WF folders/files. This could be included by
 *  doing a strcmp when looping over the directories with the SDCountFilesOnCard() function.
 * These values will not change during the course of a DIR function call, so this function only needs
 *  to be called once at the beginning. From then the buffer bytes should be left intact.
 *
 * @param	(int)the SD card ID number, either 0/1
 * @param	(char *)pointer to the buffer where we are putting the header in
 *
 * @return	(int)the status variable indicating success/failure for this function
 */
int SDCreateDIRHeader( unsigned char *packet_buffer, int sd_card_number )
{
	int status = 0;
	unsigned int real_time = GetRealTime();

	packet_buffer[CCSDS_HEADER_FULL] = sd_card_number;
	packet_buffer[CCSDS_HEADER_FULL + 1] = NEWLINE_CHAR_CODE;
	packet_buffer[CCSDS_HEADER_FULL + 2] = (unsigned char)(real_time >> 24);
	packet_buffer[CCSDS_HEADER_FULL + 3] = (unsigned char)(real_time >> 16);
	packet_buffer[CCSDS_HEADER_FULL + 4] = (unsigned char)(real_time >> 8);
	packet_buffer[CCSDS_HEADER_FULL + 5] = (unsigned char)(real_time);
	packet_buffer[CCSDS_HEADER_FULL + 6] = NEWLINE_CHAR_CODE;
	packet_buffer[CCSDS_HEADER_FULL + 7] = (unsigned char)(sd_total_folders >> 24);
	packet_buffer[CCSDS_HEADER_FULL + 8] = (unsigned char)(sd_total_folders >> 16);
	packet_buffer[CCSDS_HEADER_FULL + 9] = (unsigned char)(sd_total_folders >> 8);
	packet_buffer[CCSDS_HEADER_FULL + 10] = (unsigned char)(sd_total_folders);
	packet_buffer[CCSDS_HEADER_FULL + 11] = NEWLINE_CHAR_CODE;
	packet_buffer[CCSDS_HEADER_FULL + 12] = (unsigned char)(sd_total_files >> 24);
	packet_buffer[CCSDS_HEADER_FULL + 13] = (unsigned char)(sd_total_files >> 16);
	packet_buffer[CCSDS_HEADER_FULL + 14] = (unsigned char)(sd_total_files >> 8);
	packet_buffer[CCSDS_HEADER_FULL + 15] = (unsigned char)(sd_total_files);
	packet_buffer[CCSDS_HEADER_FULL + 16] = NEWLINE_CHAR_CODE;

	return status;
}
/*
 * Function to loop over the SD card directories and count up the number of files and folders.
 * There is a distinction made between
 */
FRESULT SDCountFilesOnCard( char *path )
{
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

	res = f_opendir(&dir, path);
	if (res == FR_OK) {
		for (;;)
		{
			res = f_readdir(&dir, &sd_fno);							/* Read a directory item */
			if (res != FR_OK || sd_fno.fname[0] == 0)break;			/* Break on error or end of dir */
			if (sd_fno.fname[0] == '.') continue;					/* Ignore the dot entry */
			if (sd_fno.fattrib & AM_HID) continue;					/* Ignore hidden directories */
			if (sd_fno.fattrib & AM_SYS) continue;					/* Ignore system directories */
			//check what type of name we should use
			fn = *sd_fno.lfname ? sd_fno.lfname : sd_fno.fname;
			if (sd_fno.fattrib & AM_DIR)
			{
				i = strlen(path);
				sprintf(&path[i], "/%s", fn);
				sd_count_folders++;
				res = SDCountFilesOnCard(path);						/* Enter the directory */
				if (res != FR_OK)
					break;
				path[i] = 0;
			}
			else
				sd_count_files++;
		}
		f_closedir(&dir);
	}

	//validate our new number against what the system has recorded from creation and deletion:
	//The real question is: which number do we believe if they disagree?
	// My guess would be to believe this number, I suppose...
	if(sd_count_folders != sd_total_folders)
	{
		sd_total_folders = sd_count_folders;
		SetSDTotalFolders(sd_count_folders);
	}

	if(sd_count_files != sd_total_files)
	{
		sd_total_files = sd_count_files;
		SetSDTotalFiles(sd_count_files);
	}

	return res;
}

/*
 * Function to scan the contents of the Root directory and all folders on the Root directory.
 *
 * @param	path to the directory to be scanned
 * 			use this command with "0" or "1" to scan the entire Root directory
 * @param	(char *)pointer to the buffer to use for the DIR packets
 * 				Since this function is called recursively, we must pass this through so the same
 * 				buffer is used, rather than multiple buffers.
 *
 * @return	(FRESULT) SD card library status indicator; use this to jump back from directories when looping
 * 				If this is FR_OK, we finished successfully
 * 				If this is not FR_OK, there was an error somewhere; depending on the error it could have
 * 				 been from either f_opendir or f_readdir
 */
FRESULT SDScanFilesOnCard( char *path, unsigned char *packet_buffer )
{
	/*
	 * This function is taken from the elm-chan website: http://elm-chan.org/fsw/ff/doc/readdir.html
	 * I have changed the function from the website so that we can pull the folder and file names,
	 *  and the file sizes and print them to a char buffer that can be sent as a packet.
	*/
	int bytes_written = 0;
	FRESULT res;
	DIR dir;
	UINT i;
	char *fn;
	sd_fno.lfname = sd_LFName;
	sd_fno.lfsize = sizeof(sd_LFName);

	//when we get to this point, we already have a CCSDS header on the packet buffer. We will need to modify the group flags before sending
	//we also just counted the total number of files and folders on the SD card
	//we can start writing into the buffer at location buff[11] which is the first data byte

	res = f_opendir(&dir, path);
	if (res == FR_OK) {
		for (;;)
		{
			res = f_readdir(&dir, &sd_fno);
			if (res != FR_OK || sd_fno.fname[0] == 0)break;	/* Break on error or end of dir */
			if (sd_fno.fname[0] == '.') continue;			/* Ignore the dot entry */
			if (sd_fno.fattrib & AM_HID) continue;			/* Ignore hidden directories */
			if (sd_fno.fattrib & AM_SYS) continue;			/* Ignore system directories */
			//check what type of name we should use
			fn = *sd_fno.lfname ? sd_fno.lfname : sd_fno.fname;
			if (sd_fno.fattrib & AM_DIR)
			{
				i = strlen(path);
				sprintf(&path[i], "/%s", fn);
				//DAQ folders are 11 char long, there is a backslash and a newline added on the end = 13 bytes; snprintf prints n-1 char, so add one more = 14 bytes (potential max)
				bytes_written = snprintf((char *)&packet_buffer[CCSDS_HEADER_PRIM + DIR_PACKET_HEADER + iter], 14, "%s/\n", fn);
				if(bytes_written == 0)
					res = 20;	//unused SD card library error code //TODO: check if this is valid, handle this better
				else if(bytes_written == DAQ_FOLDER_SIZE + 2)
					iter += (DAQ_FOLDER_SIZE + 2);
				else if(bytes_written == WF_FOLDER_SIZE + 2)
					iter += (WF_FOLDER_SIZE + 2);

				res = SDScanFilesOnCard(path, packet_buffer);				/* Enter the directory */
				if (res != FR_OK)
					break;
				path[i] = 0;
			}
			else
			{
//				xil_printf("%s/%s - %u\n", path, fn, sd_fno.fsize); //print the SD card number (0/1) in the header of the packet
				bytes_written = snprintf((char *)&packet_buffer[CCSDS_HEADER_PRIM + DIR_PACKET_HEADER + iter], 17, "%s\t%d\n", fn, sd_fno.fsize);
			}
		}
		f_closedir(&dir);
	}

	return res;
}
