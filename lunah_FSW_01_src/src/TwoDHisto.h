/*
 * TwoDHisto.h
 *
 *  Created on: Feb 4, 2019
 *      Author: gstoddard
 */

#ifndef SRC_TWODHISTO_H_
#define SRC_TWODHISTO_H_

#include "lunah_defines.h"

//function prototypes
int Tally2DH(double energy_value, double psd_value, unsigned int pmt_ID);
unsigned int Get2DHArrayIndexX( double energy_value );
unsigned int Get2DHArrayIndexY( double psd_value );

#endif /* SRC_TWODHISTO_H_ */
