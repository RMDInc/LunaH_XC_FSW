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

//File Scope Variables and Buffers
static unsigned char event_buffer[EVENT_BUFFER_SIZE];


/*
 * Will move this function to a separate source/header file later
 *
 */
int Process2DHData( void )
{
	//variables for data processing
	int i_dataarray_index = 0;
	int i_PMT_ID = 0;

	int i_xnumbins = TWODH_X_BINS;
	int i_ynumbins = TWODH_Y_BINS;
	float f_xbinnum = 0;
	float f_ybinnum = 0;
//	int i_xArrayIndex = 0;
//	int i_yArrayIndex = 0;
	int badEvents = 0;
	int i_pointInsideBounds = 0;
	int i_pointOutsideBounds = 0;

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
//						i_xArrayIndex = (int)f_xbinnum;
//						i_yArrayIndex = (int)f_ybinnum;
						//increment the bin in the matrix
						switch(i_PMT_ID)
						{
							case 1:
//								++twoDH_pmt1[i_xArrayIndex][i_yArrayIndex];
								break;
							case 2:
//								++twoDH_pmt2[i_xArrayIndex][i_yArrayIndex];
								break;
							case 3:
//								++twoDH_pmt3[i_xArrayIndex][i_yArrayIndex];
								break;
							case 4:
//								++twoDH_pmt4[i_xArrayIndex][i_yArrayIndex];
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


/*
 * This function will be called after we read in a buffer of valid data from the FPGA.
 *  Here is where the data stream from the FPGA is scanned for events and each event
 *  is processed to pull the PSD and energy information out. We identify it the event
 *  is within the current 1 second CPS interval, as well as bin the events into a
 *  2-D histogram which is reported at the end of a run.
 *
 * @param	A pointer to the data buffer
 *
 * @return	SUCCESS/FAILURE
 */
int ProcessData( unsigned int * data_raw )
{
	int iter = 0;
//	int cps_iter = 0;
	int evt_iter = 0;
	unsigned int current_time = 0;
	unsigned int num_bytes_written = 0;
	FIL cpsDataFile;
	FRESULT f_res = FR_OK;
	GENERAL_EVENT_TYPE event_holder = {};
	//we have access to our raw data now
	// switch on the different "Sign post" identifiers we have

	//there are 4096 integers in the buffer that was passed in
	while(iter < 4096)
	{
		switch(data_raw[iter])
		{
		case 111111:
			//this is the data event case

			//Check on CPS report
			if(cpsCheckTime(current_time) == TRUE)
			{
				f_res = f_write(&cpsDataFile, (char *)cpsGetEvent(), CPS_EVENT_SIZE, &num_bytes_written);
				if(num_bytes_written != CPS_EVENT_SIZE)
				{
					//handle error with writing
				}
				else if(f_res != FR_OK)
				{
					//handle bad write
				}
			}

			//begin processing the event

			break;
		case 892270675:
			//this is a temperature event
			break;
		case 2147594759:
			//this is a false event
			//check to make sure this is a valid event
			if( data_raw[iter + 1] == 2147594759 && data_raw[iter + 9] == 111111)
			{
				cpsSetFirstEventTime(data_raw[iter + 2]);

				event_holder.field0 = 0xDD;
				event_holder.field1 = 0xDD;
				event_holder.field2 = 0xDD;
				event_holder.field3 = 0xDD;
				event_holder.field4 = 0x00 | (data_raw[iter + 2] >> 30);
				event_holder.field5 = data_raw[iter + 2] >> 22;
				event_holder.field6 = data_raw[iter + 2] >> 14;
				event_holder.field7 = data_raw[iter + 2] >> 6;

				memcpy(event_buffer + EVTS_EVENT_SIZE * evt_iter, &event_holder, sizeof(event_holder));	//write one event to the array
			}
			else
				break;	//not a valid first event

			break;
		default:
			//this indicates that we miscounted our place in the buffer somewhere
			//or there is junk in the buffer in the middle of an event
			//TODO: handle a bad event or junk in the buffer
			break;
		}//END OF SWITCH ON RAW DATA
	}//END OF WHILE

	return 0;
}
