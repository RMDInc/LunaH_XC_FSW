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
static unsigned short m_neutrons_ellipse1;			//neutrons with PSD
static unsigned short m_neutrons_ellipse2;			//neutrons wide cut
static unsigned short m_neutrons_with_PSD;			//neutrons with PSD for CPS tallies
static unsigned short m_neutrons_wide_cut;			//neutrons within the second cut box
static unsigned short m_neutrons_no_PSD;			//all events within an energy range, no PSD cut applied
static unsigned short m_events_over_threshold;		//count all events which trigger the system

static XTime cps_t_elapsed;	//was LocalTime
static XTime cps_t_next_interval;	//added to take the place of t_elapsed
static XTime cps_t_start;	//was LocalTimeStart
static XTime cps_t_current;	//was LocalTimeCurrent

static unsigned int m_current_module_temp;

static double a_rad_1[4];		//semi-major axis
static double b_rad_1[4];		//semi-minor axis
static double mean_psd_1[4];	//Y center of the ellipse
static double mean_nrg_1[4];	//X center of the ellipse
static double a_rad_2[4];
static double b_rad_2[4];
static double mean_psd_2[4];
static double mean_nrg_2[4];

//Temperature Correction Value Arrays
//2-D arrays example: my_array[row][column];
static double MinNRG_C0[2][4] = {{ 107.88,  105.47,   102.23,   104.80   }, {  89.283,   102.45,   93.524,  99.865   }};
static double MinNRG_C1[2][4] = {{   2.4292,  2.1978,   2.2977,   2.7017 }, {   1.6472,   1.9591,   2.8101,  2.1276  }};
static double MaxNRG_C0[2][4] = {{ 143.60,  140.30,   142.51,   149.89   }, { 134.67,   140.69,   147.52,  148.08    }};
static double MaxNRG_C1[2][4] = {{   2.9196,  2.1951,   2.8573,   3.5465 }, {   2.1884,   2.4109,   3.7654,  2.7265  }};
static double MinPSD_C0[2][4] = {{   0.10948, 0.13519,  0.10577,  0.15994}, {   0.19321,  0.11577,  0.11185,  0.11321}};
static double MinPSD_C1[2][4] = {{   0.00127, 9.41e-5,  0.00135,  6.56e-4}, {   5.65e-4,  0.00103,  0.00141,  0.00118}};
static double MaxPSD_C0[2][4] = {{   0.3599,  0.3627,   0.3593,   0.3764 }, {   0.4187,   0.34404,  0.3564,  0.3567  }};
static double MaxPSD_C1[2][4] = {{  -0.0022, -0.00355, -0.00255, -0.00391}, {  -0.00406, -0.00185, -0.00300, -0.00197}};
static double MaxPSD_C2[2][4] = {{   4.49e-5, 7.62e-5,  5.24e-5,  8.57e-5}, {   1.06e-4,  3.86e-5,  6.92e-5,  3.53e-5}};

static CONFIG_STRUCT_TYPE m_cfg_buff;	//172 bytes

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
	//get the neutron cuts
	m_cfg_buff = *GetConfigBuffer();

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
 * Initialize cps_t_start and cps_t_elapsed at startup
 */
void cpsInitStartTime(void)
{
	XTime_GetTime(&cps_t_start);	//get the time
	cps_t_next_interval = 0;
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

/*
 * Helper function which takes in the energy, psd, module number, and the ellipse numbers and calculates the
 *  equations for the ellipses. Then it does the comparison to see if the point (energy, psd) is within the
 *  bounding ellipses.
 */
bool CPSIsWithinEllipse( double energy, double psd, int module_num, int ellipse_num )
{
	bool ret = FALSE;
	float c1 = m_cfg_buff.SF_PSD[ellipse_num] * b_rad_1[module_num] / m_cfg_buff.SF_E[ellipse_num] * a_rad_1[module_num];
	float c2 = m_cfg_buff.SF_E[ellipse_num] 	* a_rad_1[module_num];
	float xval = energy - mean_nrg_1[module_num] - m_cfg_buff.Off_E[ellipse_num];
	float yval = psd	- mean_psd_1[module_num] - m_cfg_buff.Off_PSD[ellipse_num];

	if(	yval <  c1 * sqrt( c2 * c2 - xval * xval) && yval > -c1 * sqrt( c2 * c2 - xval * xval))
		ret = TRUE;
	else
		ret = FALSE;

	return ret;
}

/*
 * Access function to update the tallies that we add each time we process an event. We store the
 *  various neutron totals in this module and use this function to update them. This function has access
 *  to the static neutron totals in this module and adds to them after running the input energy and psd
 *  value through the neutron cut values.
 * The neutron cut values are set and changed via the MNS_NGATES command.
 * This function gets the values from the temperature sensors every 10s and uses those temperatures
 *  to move the cutting ellipses around within the run. This way, we can maintain good data
 *  acquisition during a run when the temperature is not guaranteed to be constant.
 *
 * This approach moves away from the previously defined "box" cuts. The values and ranges from using
 *  that approach will be removed once this method is implemented and tested.
 *
 * The first time that this function is visited, the neutron cuts will be calculated, then once every 10s
 *  the module temperature will be re-measured and the cuts will be re-calculated.
 *
 * This function is going to use the current temperature of the module temperature sensor to calculate
 *  the neutron cuts. This should give us a good enough approximation of what the correct temperature is.
 *  In the next iteration, it would be good to utilize an array of the temperatures from the module
 *  temperature sensor. This way we can use the temperature which is from the actual time the data was taken
 *  and not just the current sensor temperature, which could be more recent than the data was taken in
 *  a low rate environment.
 *
 * NB: The wide	cut ellipse is currently hard-coded to be 20% larger in each dimension than the first ellipse.
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
int CPSUpdateTallies(double energy, double psd, int pmt_id)
{
	int iter = 0;
	int m_neutron_detected = 0;
	int model_id_num = 0;
	int ell_1 = 0;	//ellipse 1
	int ell_2 = 0;	//ellipse 2
	float MaxNRG = 0;
	float MinNRG = 0;
	float MaxPSD = 0;
	float MinPSD = 0;
	//check the temperature if it has been ~10s
	XTime_GetTime(&cps_t_current);
	cps_t_elapsed = (cps_t_current - cps_t_start)/COUNTS_PER_SECOND;
	if(cps_t_elapsed >= cps_t_next_interval)
	{
		while(cps_t_elapsed >= cps_t_next_interval)
		{
			cps_t_next_interval += 10;	//this value is how long we will wait in between checks on the temperature
		}
		m_current_module_temp = GetAnlgTemp(); //= GetModuTemp();	//TODO: use the module temp sensor when I attach one...

		//Calculate the values for the cuts to used
		//This is based on the temperature, which is the driver for where the cuts should be.
		//The other driver is the module number, as each module has different cuts.
		//basic equation:
		// E   = table_val0 + table_val1*temp^1
		// PSD = table_val0 + table_val1*temp^1 + table_val2*temp^2
		//calculate the ellipse parameters
		for(iter = 0; iter < 4; iter++)
		{
			MinNRG = MinNRG_C0[MNS_DETECTOR_NUM][iter] + MinNRG_C1[MNS_DETECTOR_NUM][iter]*m_current_module_temp;
			MaxNRG = MaxNRG_C0[MNS_DETECTOR_NUM][iter] + MaxNRG_C1[MNS_DETECTOR_NUM][iter]*m_current_module_temp;
			MinPSD = MinPSD_C0[MNS_DETECTOR_NUM][iter] + MinPSD_C1[MNS_DETECTOR_NUM][iter]*m_current_module_temp;
			MaxPSD = MaxPSD_C0[MNS_DETECTOR_NUM][iter] + MaxPSD_C1[MNS_DETECTOR_NUM][iter]*m_current_module_temp + MaxPSD_C2[MNS_DETECTOR_NUM][iter]*m_current_module_temp*m_current_module_temp;

			MinNRG *= 0.8;	//random extra scaling
			MaxNRG *= 1.2;
			//calculate the parameters
			//will need to modify these parameters with the scale factor & offset values from setIntstrumentParams
			a_rad_1[iter] =	 (MaxNRG - MinNRG) / 2.0;	// a, semi-major axis
			b_rad_1[iter] =	 (MaxPSD - MinPSD) / 2.0;	// b, semi-minor axis
			mean_nrg_1[iter] = (MaxNRG + MinNRG) / 2.0;	// X center
			mean_psd_1[iter] = (MaxPSD + MinPSD) / 2.0;	// Y center

			//find the values for the larger ellipse (if we're doing a statically larger ellipse ~20% or something)
			//find the values for: mean_psd_2, mean_nrg_2, brad_2, arad_2
			a_rad_2[iter] =	 (MaxNRG - MinNRG) / 2.0;
			b_rad_2[iter] =	 (MaxPSD - MinPSD) / 2.0;
			mean_psd_2[iter] = (MaxNRG + MinNRG) / 2.0;
			mean_nrg_2[iter] = (MaxPSD + MinPSD) / 2.0;
		}
	}
	//compare energy, psd values to the cuts //tally if inside, otherwise no tally
	///////////
	//NOTE: if the pmt ID number is not a single hit, then we won't add it to the tallies
	///////////
	if(pmt_id == PMT_ID_0 || pmt_id == PMT_ID_1 || pmt_id == PMT_ID_2 || pmt_id == PMT_ID_3)
	{
		switch(pmt_id)
		{
		case PMT_ID_0:
			model_id_num = 0;
			ell_1 = 0;
			ell_2 = 1;
			break;
		case PMT_ID_1:
			model_id_num = 1;
			ell_1 = 2;
			ell_2 = 3;
			break;
		case PMT_ID_2:
			model_id_num = 2;
			ell_1 = 4;
			ell_2 = 5;
			break;
		case PMT_ID_3:
			model_id_num = 3;
			ell_1 = 6;
			ell_2 = 7;
			break;
		}

		if(CPSIsWithinEllipse(energy, psd, model_id_num, ell_1))
		{
			m_neutrons_ellipse1++;
			cpsEvent.n_with_PSD_MSB = (unsigned char)(m_neutrons_ellipse1 >> 8);
			cpsEvent.n_with_PSD_LSB = (unsigned char)(m_neutrons_ellipse1);
			m_neutron_detected = 1;
		}
		//now calculate the second ellipse and take those cuts:
		if(CPSIsWithinEllipse(energy, psd, model_id_num, ell_2))
		{
			m_neutrons_ellipse2++;
			cpsEvent.n_wide_cut_MSB = (unsigned char)(m_neutrons_ellipse2 >> 8);
			cpsEvent.n_wide_cut_LSB = (unsigned char)(m_neutrons_ellipse2);
			m_neutron_detected = 1;
		}
		//does the event fit into the no PSD cut?
		if(energy >= MinNRG)
		{
			if(energy <= MaxNRG)
			{
				m_neutrons_no_PSD++;
				cpsEvent.n_with_no_PSD_MSB = (unsigned char)(m_neutrons_no_PSD >> 8);
				cpsEvent.n_with_no_PSD_LSB = (unsigned char)(m_neutrons_no_PSD);
			}
		}
	}
	//also collect the values for neutrons with energy greater than 10 MeV
	if(energy > TWODH_ENERGY_MAX)	//this will eventually be something like ConfigBuff.parameter
	{
		m_events_over_threshold++;
		cpsEvent.high_energy_events_MSB = (unsigned char)(m_events_over_threshold >> 8);
		cpsEvent.high_energy_events_LSB = (unsigned char)(m_events_over_threshold);
	}

	return m_neutron_detected;
}
