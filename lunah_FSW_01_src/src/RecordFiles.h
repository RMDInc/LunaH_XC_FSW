/*
 * RecordFiles.h
 *
 *  Created on: Aug 23, 2019
 *      Author: gstoddard
 */

#ifndef SRC_RECORDFILES_H_
#define SRC_RECORDFILES_H_

#include <stdio.h>
#include "xil_printf.h"
#include "ff.h"
#include "lunah_defines.h"

#define NO_TX_CHANGE			-1
#define MAX_FILENAME_SIZE		14
#define NUM_FILES_PER_FOLDER	25

typedef struct{
	char filename[MAX_FILENAME_SIZE];
	unsigned int total_file_size;
	unsigned int bytes_transferred;
}TX_BYTES_FILE_TYPE;

void sd_createTXBytesFile( void );
void sd_updateFileRecords( char * filename, int transfer_byteVal );
void sd_deleteFileRecord( char * filename );
int sd_totalFoldersIncrement( void );
int sd_totalFoldersDecrement( void );
int sd_getTotalFolders( void );
void sd_setTotalFolders( int num_folders );
int sd_totalFilesIncrement( void );
int sd_totalFilesDecrement( void );
int sd_getTotalFiles( void );
void sd_setTotalFiles( int num_folders );

#endif /* SRC_RECORDFILES_H_ */
