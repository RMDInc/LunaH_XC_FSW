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
	char filename[14];
	unsigned int total_file_size;
	unsigned int bytes_transferred;
}TX_BYTES_FILE_TYPE;

void sd_createTXBytesFile( void );
void sd_updateTXfile( char * filename, int transfer_byteVal );

#endif /* SRC_RECORDFILES_H_ */
