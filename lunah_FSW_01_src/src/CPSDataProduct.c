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
static unsigned int m_current_1sec_interval_time;	//the current 1 second interval "start" time
static CPS_EVENT_STRUCT_TYPE cpsEvent;

//Functions
void cpsSetFirstEventTime( unsigned int time )
{
	first_FPGA_time = time;
	return;
}

void cpsSetRecordedTime( unsigned int m_recorded_time )
{
	m_current_1sec_interval_time = m_recorded_time;
	return;
}

unsigned int cpsGetCurrentTime( void )
{
	return m_current_1sec_interval_time;
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
	return (time * (float)0.000262144);
}

/*
 * Helper function to compare the time of the event which was just read in
 *  to the time which defined the start of our last 1 second interval.
 *
 * @param	The FPGA time from the event
 *
 * @return	TRUE if we are past 1 second in the interval, FALSE if not
 */
bool cpsCheckTime( unsigned int time )
{
	bool mybool = FALSE;

	if (convertToSeconds(time) >= (convertToSeconds(m_current_1sec_interval_time) + 1.0))
	{
		mybool = TRUE;
		m_previous_1sec_interval_time = m_current_1sec_interval_time;
		m_current_1sec_interval_time = time;
	}
	else
		mybool = FALSE;

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
	cpsEvent.event_id = 0xAA;
	cpsEvent.time_MSB = (unsigned char)(m_previous_1sec_interval_time >> 24);
	cpsEvent.time_LSB1 = (unsigned char)(m_previous_1sec_interval_time >> 16);
	cpsEvent.time_LSB2 = (unsigned char)(m_previous_1sec_interval_time >> 8);
	cpsEvent.time_LSB3 = (unsigned char)(m_previous_1sec_interval_time);
	cpsEvent.modu_temp = (unsigned char)GetModuTemp();

	return &cpsEvent;
}
