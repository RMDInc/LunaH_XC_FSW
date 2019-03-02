/*
 * TwoDHisto.c
 *
 *  Created on: Feb 4, 2019
 *      Author: gstoddard
 */

#include "TwoDHisto.h"

/*
 * Takes energy and PSD values from an event and tallies them into 2-D Histograms.
 * This function implements the elliptical neutron cuts to determine if an event
 * was good or not.
 * The PMT ID is a parameter so that we can tally the appropriate histograms, as
 *  well as tally the total, at the same time and in one function.
 *
 * @param	The calculated energy of the event
 * @param	The calculated PSD ratio of the event
 * @param	The PMT ID from the event
 *
 * @return	SUCCESS/FAILURE
 */
int Tally2DH(double energy_value, double psd_value, unsigned int pmt_ID)
{
	int status = CMD_FAILURE;
//	double m_energy_bin = 0.0;
//	double m_psd_bin = 0.0;
//	double m_x_bin_size = TWODH_ENERGY_MAX / TWODH_X_BINS;
//	double m_y_bin_size = TWODH_PSD_MAX / TWODH_Y_BINS;

	//find the bin numbers
//	m_energy_bin = energy_value / m_x_bin_size;
//	m_psd_bin = psd_value / m_y_bin_size;

	//validate the bin numbers, then increment the bin in the end-of-run 2DH
/*	if(0 <= m_energy_bin && energy_bin <= (TWODH_X_BINS - 1))
		if(0 <= m_psd_bin && m_psd_bin <= (TWODH_Y_BINS - 1))
		{
			switch(pmt_ID)
			{
			case 1:
				++m_2DH_pmt1[(int)m_energy_bin][(int)m_psd_bin];
				break;
			case 2:
				++m_2DH_pmt2[(int)m_energy_bin][(int)m_psd_bin];
				break;
			case 4:
				++m_2DH_pmt3[(int)m_energy_bin][(int)m_psd_bin];
				break;
			case 8:
				++m_2DH_pmt4[(int)m_energy_bin][(int)m_psd_bin];
				break;
			default:
				//TODO: handle non-singleton hits
				break;
			}
		} */

	return status;
}

/*
 * Calculates the X array index for the EVTs data product
 *
 * @param	None
 *
 * @return	(int) bin number to be stored in an EVTs event
 */
unsigned int Get2DHArrayIndexX( double energy_value )
{
	unsigned int m_bin_number = 0;

	//this line is bothersome, as I want to floor the value, but then have to cast it anyway...
	m_bin_number = (unsigned int)floor(energy_value / ((double)TWODH_ENERGY_MAX / (double)TWODH_X_BINS));

	if(0 <= m_bin_number && m_bin_number < TWODH_X_BINS)
		m_bin_number &= 0x03FF;
	else
		m_bin_number = 0x0103;

	return m_bin_number;
}

/*
 * Calculates the Y array index for the EVTs data product
 *
 * @param	None
 *
 * @return	(int) bin number to be stored in an EVTs event
 */
unsigned int Get2DHArrayIndexY( double psd_value )
{
	unsigned int m_bin_number = 0;

	//this line is bothersome, as I want to floor the value, but then have to cast it anyway...
	m_bin_number = (unsigned int)floor(psd_value / ((double)TWODH_PSD_MAX / (double)TWODH_Y_BINS));

	if(0 <= m_bin_number && m_bin_number < TWODH_Y_BINS)
		m_bin_number &= 0x3F;
	else
		m_bin_number = 0x1D;

	return m_bin_number;
}
