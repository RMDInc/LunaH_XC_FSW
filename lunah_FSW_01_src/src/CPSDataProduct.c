/*
 * CPSDataProduct.c
 *
 *  Created on: Jan 18, 2019
 *      Author: gstoddard
 */

#include "CPSDataProduct.h"

//File-Scope Variables
static unsigned int first_FPGA_time;				//the first FPGA time we register for the run //sync with REAL TIME
static unsigned int m_previous_1sec_interval_time;	//the previous 1 second interval "start" time
static float m_num_intervals_elapsed;				//how many intervals have elapsed during the current run (effectively one/sec)
static CPS_EVENT_STRUCT_TYPE cpsEvent;				//the most recent CPS "event" (1 second of counts)
static const CPS_EVENT_STRUCT_TYPE cpsEmptyStruct;	//an empty 'zero' struct to init or clear other structs
static unsigned short m_neutrons_ellipse1;		//neutrons with PSD
static unsigned short m_neutrons_ellipse2;		//neutrons wide cut
static unsigned short m_neutrons_with_PSD;		//neutrons with PSD for CPS tallies
static unsigned short m_neutrons_wide_cut;		//neutrons within the second cut box
static unsigned short m_neutrons_no_PSD;			//all events within an energy range, no PSD cut applied
static unsigned short m_events_over_threshold;	//count all events which trigger the system

static CPS_BOX_CUTS_STRUCT_TYPE m_cps_box_cuts;

//Functions
//TODO: remove this function once the ellipse cuts are implemented
void CPSSetCuts( void )
{
	//if the set cuts value is 0, then grab defaults, otherwise user-defined values are in the struct
	m_cps_box_cuts = *GetCPSCutVals();
	if(m_cps_box_cuts.set_cuts == 0)
	{
		m_cps_box_cuts.ecut_low = CPS_ECUT_LOW;
		m_cps_box_cuts.ecut_high = CPS_ECUT_HIGH;
		m_cps_box_cuts.pcut_low = CPS_PCUT_LOW;
		m_cps_box_cuts.pcut_high = CPS_PCUT_HIGH;
		m_cps_box_cuts.ecut_wide_low = CPS_ECUT_WIDE_LOW;
		m_cps_box_cuts.ecut_wide_high = CPS_ECUT_WIDE_HIGH;
		m_cps_box_cuts.pcut_wide_low = CPS_PCUT_WIDE_LOW;
		m_cps_box_cuts.pcut_wide_high = CPS_PCUT_WIDE_HIGH;
	}
	else if(m_cps_box_cuts.set_cuts == 1)
	{
		//check that none of the params are 0, if half of the cuts (normal or wide) are not set, just take the percentage
		if(m_cps_box_cuts.ecut_low && m_cps_box_cuts.ecut_high && m_cps_box_cuts.pcut_low && m_cps_box_cuts.pcut_high)
		{
			m_cps_box_cuts.ecut_low = m_cps_box_cuts.ecut_wide_low * 0.8;
			m_cps_box_cuts.ecut_high = m_cps_box_cuts.ecut_wide_high * 0.8;
			m_cps_box_cuts.pcut_low = m_cps_box_cuts.pcut_wide_low * 0.8;
			m_cps_box_cuts.pcut_high = m_cps_box_cuts.pcut_wide_high * 0.8;
		}
		else if(m_cps_box_cuts.ecut_wide_low && m_cps_box_cuts.ecut_wide_high && m_cps_box_cuts.pcut_wide_low && m_cps_box_cuts.pcut_wide_high)
		{
			m_cps_box_cuts.ecut_wide_low = m_cps_box_cuts.ecut_low * 1.2;
			m_cps_box_cuts.ecut_wide_high = m_cps_box_cuts.ecut_high * 1.2;
			m_cps_box_cuts.pcut_wide_low = m_cps_box_cuts.pcut_low * 1.2;
			m_cps_box_cuts.pcut_wide_high = m_cps_box_cuts.pcut_high * 1.2;
		}
	}

	return;
}


/*
 * Reset the counts per second data product counters and event structures for the run.
 * Call this function before each DAQ run.
 *
 * @return	none
 *
 */
void CPSInit( void )
{
	first_FPGA_time = 0;
	m_previous_1sec_interval_time = 0;
	m_num_intervals_elapsed = 0;
	cpsEvent = cpsEmptyStruct;
	m_neutrons_ellipse1 = 0;
	m_neutrons_ellipse2 = 0;
	m_neutrons_with_PSD = 0;
	m_neutrons_wide_cut = 0;
	m_neutrons_no_PSD = 0;
	m_events_over_threshold = 0;
	return;
}

void CPSResetCounts( void )
{
	m_neutrons_with_PSD = 0;	//reset the values from processing
	m_neutrons_wide_cut = 0;
	m_neutrons_no_PSD = 0;
	m_events_over_threshold = 0;
	cpsEvent.n_with_PSD_MSB = 0;//reset values in the struct we report
	cpsEvent.n_with_PSD_LSB = 0;
	cpsEvent.n_wide_cut_MSB = 0;
	cpsEvent.n_wide_cut_LSB = 0;
	cpsEvent.n_with_no_PSD_MSB = 0;
	cpsEvent.n_with_no_PSD_LSB = 0;
	cpsEvent.high_energy_events_MSB = 0;
	cpsEvent.high_energy_events_LSB = 0;
	return;
}

void cpsSetFirstEventTime( unsigned int time )
{
	first_FPGA_time = time;
	return;
}

unsigned int cpsGetFirstEventTime( void )
{
	return first_FPGA_time;
}

unsigned int cpsGetCurrentTime( void )
{
	return (convertToSeconds(first_FPGA_time) + m_num_intervals_elapsed);
}

/*
 * Helper function to convert the FPGA time from clock cycles to seconds
 *
 * @param	The integer time from the FPGA
 *
 * @return	The converted time in seconds
 */
float convertToSeconds( unsigned int time )
{
	return ((float)time * 0.000262144);
}

/*
 * Helper function to convert the number of elapsed 1s intervals into clock cycles.
 * This function converts from seconds -> clock cycles, then drops off any remainder
 *  by casting to unsigned int.
 *
 * @param	the floating point time to convert
 *
 * @return	the converted number of cycles
 */
unsigned int convertToCycles( float time )
{
	return (unsigned int)(time / 0.000262144);
}

/*
 * Helper function to compare the time of the event which was just read in
 *  to the time which defined the start of our last 1 second interval.
 * This will get called every time we get a full buffer and there is valid
 *  data within the buffer. When that happens, this will compare the time
 *  from the event to the current 1 second interval to see if the event falls
 *  within that time frame. If it does, move on and add the counts to the interval.
 *  If it does not fall within the interval, record that CPS event and go to
 *  the next one. Continue this process until the time falls within an interval.
 *
 * @param	The FPGA time from the event
 *
 * @return	TRUE if we need to record the CPS event, FALSE if not
 * 			A return value of TRUE will call this function again.
 */
bool cpsCheckTime( unsigned int time )
{
	bool mybool = FALSE;

	//for this function, we define the 1 second intervals for the entire run off
	// of the first event time that comes in from the FPGA
	//thus, if the event is within the interval defined by first_evt_time -> first_evt_time + 1.0
	// then it should be included with that CPS event
	//otherwise, report that 1 second interval and move to the next interval,
	// then check if the event goes into that interval
	//repeat this process until an interval is found
	//Intervals with 0 events in them are still valid
	if(convertToSeconds(time) >= (convertToSeconds(first_FPGA_time) + m_num_intervals_elapsed))
	{
		//this means that it does not fall within the current 1s interval
		//record the start time of this interval
		m_previous_1sec_interval_time = first_FPGA_time + convertToCycles(m_num_intervals_elapsed);
		//increase the number of intervals elapsed
		m_num_intervals_elapsed++;
		mybool = TRUE;
	}
	else
		mybool = FALSE;	//still within the current interval

	return mybool;
}

/*
 * Getter function for retrieving the most recent CPS "event". This function
 *  returns a pointer to the struct after updating it with the most up-to-date
 *  information regarding the DAQ run.
 *
 * @param	None
 *
 * @return	Pointer to a CPS Event held in a struct
 */
CPS_EVENT_STRUCT_TYPE * cpsGetEvent( void )
{
	cpsEvent.event_id = 0x55;	//use the APID for CPS
	cpsEvent.time_MSB = (unsigned char)(m_previous_1sec_interval_time >> 24);
	cpsEvent.time_LSB1 = (unsigned char)(m_previous_1sec_interval_time >> 16);
	cpsEvent.time_LSB2 = (unsigned char)(m_previous_1sec_interval_time >> 8);
	cpsEvent.time_LSB3 = (unsigned char)(m_previous_1sec_interval_time);
	cpsEvent.modu_temp = (unsigned char)GetModuTemp();

	return &cpsEvent;
}


//	 * unsigned short m_neutrons_ellipse1;		//neutrons with PSD
//	 * unsigned short m_neutrons_ellipse2;		//neutrons wide cut
//	 * unsigned short m_events_noPSD;			//all events within an energy range, no PSD cut applied
//	 * unsigned short m_events_over_threshold;	//count all events which trigger the system

/*
 * Access function to update the tallies that we add each time we process an event. We store the
 *  various neutron totals in this module and use this function to update them. This function has access
 *  to the static neutron totals in this module and adds to them after running the input energy and psd
 *  value through the neutron cut values.
 * The neutron cut values are set and changed via the MNS_NGATES command.
 *
 * NB: The NGATES command is currently (4/12/2019) not parametrized for the neutron cuts this function
 * 		cares about. This function is currently utilizing the old/simple neutron Box cutting. The wide
 * 		cut box is currently hard-coded to be 20% larger in each dimension than the first cuts.
 *
 * NB: This function has specially defined values in the header for this file. Change those values to
 * 		affect these cuts. Once we implement the elliptical cuts, we'll get rid of those values.
 *
 *
 * @param	(float) value for the energy calculated from the Full Integral from the event
 * @param	(float) value for the PSD calculated from the short and long integrals from the event
 *
 * @return	(int) returns a 1 if there was a hit in the regular or wide neutron cut boxes, returns 0 otherwise
 */
int CPSUpdateTallies(double energy, double psd)
{
	int m_neutron_detected = 0;
	//Will eventually need to include the SetInstrumentParams.h so we can access the configuration buffer
	//is the event greater than 10 MeV?
	if(energy > TWODH_ENERGY_MAX)	//this will eventually be something like ConfigBuff.parameter
	{
		m_events_over_threshold++;
		cpsEvent.high_energy_events_MSB = (unsigned char)(m_events_over_threshold >> 8);
		cpsEvent.high_energy_events_LSB = (unsigned char)(m_events_over_threshold);
	}

	//does the event fit into the no PSD cut?
	if(energy >= m_cps_box_cuts.ecut_low)
	{
		if(energy <= m_cps_box_cuts.ecut_high)
		{
			m_neutrons_no_PSD++;
			cpsEvent.n_with_no_PSD_MSB = (unsigned char)(m_neutrons_no_PSD >> 8);
			cpsEvent.n_with_no_PSD_LSB = (unsigned char)(m_neutrons_no_PSD);
		}
	}

	//check for neutrons in the primary cuts
	if(energy >= m_cps_box_cuts.ecut_low)
	{
		if(energy <= m_cps_box_cuts.ecut_high)
		{
			if(psd >= m_cps_box_cuts.pcut_low)
			{
				if(psd <= m_cps_box_cuts.pcut_high)
				{
					m_neutrons_with_PSD++;
					cpsEvent.n_with_PSD_MSB = (unsigned char)(m_neutrons_with_PSD >> 8);
					cpsEvent.n_with_PSD_LSB = (unsigned char)(m_neutrons_with_PSD);
					m_neutron_detected = 1;
				}
			}
		}
	}

	//check for neutrons in the secondary cuts
	//this number should be larger than the neutrons w/psd cut because these cuts are wider, but
	//will still catch all the neutrons from above
	if(energy >= m_cps_box_cuts.ecut_wide_low)
	{
		if(energy <= m_cps_box_cuts.ecut_wide_high)
		{
			if(psd >= m_cps_box_cuts.pcut_wide_low)
			{
				if(psd <= m_cps_box_cuts.pcut_wide_high)
				{
					m_neutrons_wide_cut++;
					cpsEvent.n_wide_cut_MSB = (unsigned char)(m_neutrons_wide_cut >> 8);
					cpsEvent.n_wide_cut_LSB = (unsigned char)(m_neutrons_wide_cut);
					m_neutron_detected = 1;
				}
			}
		}
	}

	return m_neutron_detected;
}
