/*
 * process_data.h
 *
 *  Created on: May 9, 2018
 *      Author: gstoddard
 */

#ifndef SRC_PROCESS_DATA_H_
#define SRC_PROCESS_DATA_H_

#include "ff.h"
#include "lunah_defines.h"

//function prototype
int ProcessData(unsigned int * data_array,
		unsigned short twoDH_pmt1[TWODH_X_BINS][TWODH_Y_BINS],
		unsigned short twoDH_pmt2[TWODH_X_BINS][TWODH_Y_BINS],
		unsigned short twoDH_pmt3[TWODH_X_BINS][TWODH_Y_BINS],
		unsigned short twoDH_pmt4[TWODH_X_BINS][TWODH_Y_BINS]);

#endif /* SRC_PROCESS_DATA_H_ */
