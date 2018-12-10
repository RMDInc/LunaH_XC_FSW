/*
 * process_data.c
 *
 *  Created on: May 9, 2018
 *      Author: gstoddard
 */

#include "process_data.h"

///// Structure Definitions ////
struct event_raw {			// Structure is 8+4+8+8+8+8= 44 bytes long
	double time;
	long long total_events;
	long long event_num;
	double bl;
	double si;
	double li;
	double fi;
	double psd;
	double energy;
};

struct cps_data {
	unsigned short n_psd;
	unsigned short counts;
	unsigned short n_no_psd;
	unsigned short n_wide_cut;
	unsigned int time;
	unsigned char temp;
};

struct event_by_event {
	unsigned short u_EplusPSD;
	unsigned int ui_localTime;
	unsigned int ui_nEvents_temp_ID;
};

struct counts_per_second {
	unsigned char uTemp;
	unsigned int ui_nPSD_CNTsOverThreshold;
	unsigned int ui_nNoPSD_nWideCuts;
	unsigned int ui_localTime;
};

struct twoDHisto {
	unsigned int greatestBinVal;
	unsigned char numXBins;
	unsigned char numYBins;
	unsigned char xRangeMax;
	unsigned char yRangeMax;
	unsigned short twoDHisto[25][78];
};

int process_data(unsigned int * data_array,
		unsigned short twoDH_pmt1[TWODH_X_BINS][TWODH_Y_BINS],
		unsigned short twoDH_pmt2[TWODH_X_BINS][TWODH_Y_BINS],
		unsigned short twoDH_pmt3[TWODH_X_BINS][TWODH_Y_BINS],
		unsigned short twoDH_pmt4[TWODH_X_BINS][TWODH_Y_BINS])
{
	//have the data we need to process in data_array, there are 512*4 events
	//get access to buffers we will use to sort/process the data into
	//will need to create the buffers and pass in references to them (pointers)
	//variables for data processing
	int i_dataarray_index = 0;
//	int i_total_events = 0;
//	int i_event_number = 0;
	int i_PMT_ID = 0;
//	float f_time = 0;
//	float f_aa_bl_int = 0;
//	float f_aa_short_int = 0;
//	float f_aa_long_int = 0;
//	float f_aa_full_int = 0;
//	float f_lpf1 = 0;
//	float f_lpf2 = 0;
//	float f_lpf3 = 0;
//	float f_dff1 = 0;
//	float f_dff2 = 0;
//	float f_bl1 = 0.0;
//	float f_bl2 = 0.0;
//	float f_bl3 = 0.0;
//	float f_bl4 = 0.0;
//	float f_bl_avg = 0.0;
//	float f_blcorr_short = 0.0;
//	float f_blcorr_long = 0.0;
//	float f_blcorr_full = 0.0;
//	float f_PSD = 0.0;
//	float f_Energy = 0.0;
	//settings needed for processing
//	float f_aa_bl_int_samples = 38.0;
//	float f_aa_short_int_samples = 73.0;
//	float f_aa_long_int_samples = 169.0;
//	float f_aa_full_int_samples = 1551.0;
//	float f_energy_slope = 1.0;
//	float f_energy_intercept = 0.0;
	//set the ranges for the 2DH
//	int i_xminrange = 0;
//	int i_xmaxrange = 10000;
//	int i_yminrange = 0;
//	int i_ymaxrange = 2;
	//set the number of bins for the 2DH
	int i_xnumbins = TWODH_X_BINS;
	int i_ynumbins = TWODH_Y_BINS;
//	float f_xbinsize = 0;
//	float f_ybinsize = 0;
	float f_xbinnum = 0;
	float f_ybinnum = 0;
	int i_xArrayIndex = 0;
	int i_yArrayIndex = 0;
	int badEvents = 0;
	int i_pointInsideBounds = 0;
	int i_pointOutsideBounds = 0;

	//determine the x and y bin size
//	f_xbinsize = (float)i_xmaxrange / i_xnumbins;
//	f_ybinsize = (float)i_ymaxrange / i_ynumbins;

	while(i_dataarray_index < DATA_BUFFER_SIZE)
	{
		//loop over the events and try and figure out the data
		//find the 111111
		//then grab the event details; bl, short, long, full, etc
		//may want to check that event num, time are increased from the previous value
		//do baseline correction
		//calculate PSD, energy
		//calculate the bin they belong to

		//sort based on PMT ID into the proper arrays
		if(0 <= f_xbinnum)	//check x bin is inside range
		{
			if(f_xbinnum <= (i_xnumbins - 1))
			{
				if(0 <= f_ybinnum)	//check y bin is inside range
				{
					if(f_ybinnum <= (i_ynumbins - 1))
					{
						//cast the bin numbers as ints so that we can use them as array indices
						i_xArrayIndex = (int)f_xbinnum;
						i_yArrayIndex = (int)f_ybinnum;
						//increment the bin in the matrix
						switch(i_PMT_ID)
						{
							case 1:
								++twoDH_pmt1[i_xArrayIndex][i_yArrayIndex];
								break;
							case 2:
								++twoDH_pmt2[i_xArrayIndex][i_yArrayIndex];
								break;
							case 3:
								++twoDH_pmt3[i_xArrayIndex][i_yArrayIndex];
								break;
							case 4:
								++twoDH_pmt4[i_xArrayIndex][i_yArrayIndex];
								break;
							default:
								++badEvents;	//increment counter of events with no PMT ID
								break;
							}

						++i_pointInsideBounds;	//the event was in our range
					}
					else
						++i_pointOutsideBounds;
				}
				else
					++i_pointOutsideBounds;
			}
			else
				++i_pointOutsideBounds;
		}
		else
			++i_pointOutsideBounds;
	}

	return 0;
}

