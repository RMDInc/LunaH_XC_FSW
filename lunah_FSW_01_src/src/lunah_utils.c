/*
 * lunah_utils.c
 *
 *  Created on: Jun 22, 2018
 *      Author: IRDLAB
 */

#include <xuartps.h>
#include "LI2C_Interface.h"
#include "lunah_defines.h"
#include "lunah_utils.h"

static XTime LocalTime = 0;
static XTime TempTime = 0;
static XTime LocalTimeStart;
static XTime LocalTimeCurrent = 0;

//may still need these if we want to 'get' the temp at some point
//also, need to verify that we are getting the correct temp
static int analog_board_temp = 25;
static int digital_board_temp = 1;
static float modu_board_temp = 25.0;
static int iNeutronTotal = 50;
static int check_temp_sensor = 0;

/*
 * Initalize LocalTimeStart at startup
 */
void InitStartTime(void)
{
	XTime_GetTime(&LocalTimeStart);	//get the time
}

XTime GetLocalTime(void)
{
	XTime_GetTime(&LocalTimeCurrent);
	LocalTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND;
	return(LocalTime);
}

XTime GetTempTime(void)
{
	XTime_GetTime(&LocalTimeCurrent);
	TempTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND;
	return(TempTime);
}

/*
 *  Stub file to return neuron total.
 */
int GetNeutronTotal(void)
{
	return(iNeutronTotal);
}

int PutNeuronTotal(int total)
{
	iNeutronTotal = total;
	return iNeutronTotal;
}

int IncNeuronTotal(int increment)
{
    iNeutronTotal += increment;
	return iNeutronTotal;
}
/*
 * Getter functions to grab the temperature which was most recently read by the system
 *
 * @param:	None
 *
 * Return:	(INT) gives the temperature from the chosen module
 */
int GetDigiTemp( void )
{
	return digital_board_temp;
}

int GetAnlgTemp( void )
{
	return analog_board_temp;
}

int GetModuTemp( void )
{
	return modu_board_temp;
}

/*
 *  CheckForSOH
 *      Check if time to send SOH and if it is send it.
 */
int CheckForSOH(XIicPs * Iic, XUartPs Uart_PS)
{
  int iNeutronTotal;

	XTime_GetTime(&LocalTimeCurrent);
	if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (LocalTime +  1))
	{
		iNeutronTotal = GetNeutronTotal();
		LocalTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND;
		report_SOH(Iic, LocalTime, iNeutronTotal, Uart_PS, GETSTAT_CMD);	//use GETSTAT_CMD for heartbeat
	}
	return LocalTime;
}



//////////////////////////// Report SOH Function ////////////////////////////////
//This function takes in the number of neutrons currently counted and the local time
// and pushes the SOH data product to the bus over the UART
int report_SOH(XIicPs * Iic, XTime local_time, int i_neutron_total, XUartPs Uart_PS, int packet_type)
{
	//Variables
	unsigned char report_buff[100] = "";
	unsigned char i2c_Send_Buffer[2] = {};
	unsigned char i2c_Recv_Buffer[2] = {};
	int a = 0;
	int b = 0;
	int status = 0;
	int bytes_sent = 0;
	int i_sprintf_ret = 0;

	i2c_Send_Buffer[0] = 0x0;
	i2c_Send_Buffer[1] = 0x0;
	int IIC_SLAVE_ADDR2 = 0x4B;	//Temp sensor on digital board
	int IIC_SLAVE_ADDR3 = 0x48;	//Temp sensor on the analog board
//	int IIC_SLAVE_ADDR5 = 0x4A;	//Extra Temp Sensor Board, on module near thermistor on TEC

	switch(check_temp_sensor){
	case 0:	//analog board
		XTime_GetTime(&LocalTimeCurrent);
		if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (TempTime + 2))
		{
			TempTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND; //temp time is reset
			check_temp_sensor++;
			IicPsMasterSend(Iic, IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, &IIC_SLAVE_ADDR3);
			IicPsMasterRecieve(Iic, i2c_Recv_Buffer, &IIC_SLAVE_ADDR3);
			a = i2c_Recv_Buffer[0]<< 5;
			b = a | i2c_Recv_Buffer[1] >> 3;
			if(i2c_Recv_Buffer[0] >= 128)
			{
				b = (b - 8192) / 16;
			}
			else
			{
				b = b / 16;
			}
			analog_board_temp = b;
		}
		break;
	case 1:	//digital board
		XTime_GetTime(&LocalTimeCurrent);
		if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (TempTime + 2))
		{
			TempTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND; //temp time is reset
			check_temp_sensor++;

			IicPsMasterSend(Iic, IIC_DEVICE_ID_1, i2c_Send_Buffer, i2c_Recv_Buffer, &IIC_SLAVE_ADDR2);
			IicPsMasterRecieve(Iic, i2c_Recv_Buffer, &IIC_SLAVE_ADDR2);
			a = i2c_Recv_Buffer[0]<< 5;
			b = a | i2c_Recv_Buffer[1] >> 3;
			if(i2c_Recv_Buffer[0] >= 128)
			{
				b = (b - 8192) / 16;
			}
			else
			{
				b = b / 16;
			}
			digital_board_temp = b;
		}
		break;
	case 2:	//module sensor
		XTime_GetTime(&LocalTimeCurrent);
		if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (TempTime + 2))
		{
			TempTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND; //temp time is reset
			check_temp_sensor = 0;
			modu_board_temp += 0.5;
		}
		break;
	default:
		status = CMD_FAILURE;
		break;
	}

	switch(packet_type)
	{
	case READ_TMP_CMD:
		//print the SOH information after the CCSDS header
		i_sprintf_ret = snprintf((char *)report_buff + 11, 100, "%d\t%d\t%2.2f\n", analog_board_temp, digital_board_temp, modu_board_temp);
		//Put in the CCSDS Header
		PutCCSDSHeader(report_buff, i_sprintf_ret, APID_TEMP);
		//calculate the checksums
		CalculateChecksums(report_buff, i_sprintf_ret);
		bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE));
		if(bytes_sent == (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE))
			status = CMD_SUCCESS;
		else
			status = CMD_FAILURE;
		break;
	case GETSTAT_CMD:
		//print the SOH information after the CCSDS header
		i_sprintf_ret = snprintf((char *)report_buff + 11, 100, "%d\t%d\t%2.2f\t%d\t%llu\n", analog_board_temp, digital_board_temp, modu_board_temp, i_neutron_total, local_time);
		//Put in the CCSDS Header
		PutCCSDSHeader(report_buff, i_sprintf_ret, APID_SOH);
		//calculate the checksums
		CalculateChecksums(report_buff, i_sprintf_ret);
		bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE));
		if(bytes_sent == (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE))
			status = CMD_SUCCESS;
		else
			status = CMD_FAILURE;
		break;
	default:
		status = CMD_FAILURE;
		break;
	}

	return status;
}

void PutCCSDSHeader(unsigned char * SOH_buff, int length, int packet_type)
{
	//get the values for the CCSDS header
	SOH_buff[0] = 0x35;
	SOH_buff[1] = 0x2E;
	SOH_buff[2] = 0xF8;
	SOH_buff[3] = 0x53;
	SOH_buff[4] = 0x0A; //identify detector 0 or 1
	//use the input to determine what APID to fill here
	switch(packet_type)
	{
	case APID_CMD_SUCC:
		SOH_buff[5] = 0x00;	//APID for SOH
		break;
	case APID_CMD_FAIL:
		SOH_buff[5] = 0x11;	//APID for temp packet
		break;
	case APID_SOH:
		SOH_buff[5] = 0x22;	//APID for SOH
		break;
	case APID_LS_FILES:
		SOH_buff[5] = 0x33;	//APID for SOH
		break;
	case APID_TEMP:
		SOH_buff[5] = 0x44;	//APID for SOH
		break;
	case APID_MNS_CPS:
		SOH_buff[5] = 0x55;	//APID for SOH
		break;
	case APID_MNS_WAV:
		SOH_buff[5] = 0x66;	//APID for SOH
		break;
	case APID_MNS_EVTS:
		SOH_buff[5] = 0x77;	//APID for SOH
		break;
	case APID_MNS_2DH:
		SOH_buff[5] = 0x88;	//APID for SOH
		break;
	case APID_LOG_FILE:
		SOH_buff[5] = 0x99;	//APID for SOH
		break;
	case APID_CONFIG:
		SOH_buff[5] = 0xAA;	//APID for SOH
		break;
	default:
		SOH_buff[5] = 0x22; //default to SOH just in case?
		break;
	}

	SOH_buff[6] = 0xC0;
	SOH_buff[7] = 0x01;
	//add in the checksums to the length
	//To calculate the length of the packet, we need to add all the bytes in the MiniNS-data
	// plus the checksums (4 bytes) plus the reset request byte (1 byte)
	// then we subtract one byte
	//ICD specifies the CCSDS packet length as: the number of bytes after the CCSDS header - 1
	length += 4;
	SOH_buff[8] = (length & 0xFF00) >> 8;
	SOH_buff[9] = length & 0xFF;
	SOH_buff[10] = 0x00;

	return;
}

/**
 * Report the SUCCESS packet for a function which was received and passed
 *
 * @param Uart_PS	Pointer to the instance of the UART which will
 * 					transmit the packet to the spacecraft.
 * @param daq_filename	A switch to turn on if we want to report the
 * 						filename that a DAQ run will use.
 * 						0: no filename
 * 						1: report filename
 * 						else: no filename
 *
 * @return	CMD_SUCCESS or CMD_FAILURE depending on if we sent out
 * 			the correct number of bytes with the packet.
 *
 * NB: Only DAQ or WF should enable the daq_filename switch, otherwise a
 * 		junk filename will be received which will at least not be relevant,
 * 		but at worst could cause a problem.
 *
 */
int reportSuccess(XUartPs Uart_PS, int report_filename)
{
	int status = 0;
	int bytes_sent = 0;
	int packet_size = 0;	//Don't record the size of the CCSDS header with this variable
	int i_sprintf_ret = 0;
	unsigned char *cmdSuccess = calloc(100, sizeof(unsigned char));
	//I should look at using a regular char buffer rather than using calloc() and free()
	PutCCSDSHeader(cmdSuccess, GetLastCommandSize(), APID_CMD_SUCC);
	//fill the data bytes
	switch(report_filename)
	{
	case 1:
		//Enabled the switch to report the filename
		//should we check to see if the last command was DAQ/WF? No for now
		//print the command information after the CCSDS header
		i_sprintf_ret = snprintf((char *)cmdSuccess + 11, 100, "%s\n", GetLastCommand());
		//check to make sure that the sizes match and we are reporting what we think we are
		if(i_sprintf_ret == GetLastCommandSize())
		{
			//we successfully wrote in the last command, record the bytes we wrote
			packet_size += i_sprintf_ret;
			//now we want to add in the new filename that is going to be used
			i_sprintf_ret = snprintf((char *)cmdSuccess + 11 + i_sprintf_ret, 100, "%s", GetFileName( DATA_TYPE_EVTS ));
			if(i_sprintf_ret == GetFileNameSize())
			{
				packet_size += i_sprintf_ret;
				status = CMD_SUCCESS;
			}
			else
				status = CMD_FAILURE;
		}
		else
			status = CMD_FAILURE;
		break;
	default:
		//Case 0 is the default so that the normal success happens
		// even if we get some weird value coming through
		//no switch to report the filename, this is a normal SUCCESS PACKET
		//print the command information after the CCSDS header
		i_sprintf_ret = snprintf((char *)cmdSuccess + 11, 100, "%s\n", GetLastCommand());
		//check to make sure that the sizes match and we are reporting what we think we are
		if(i_sprintf_ret == GetLastCommandSize())
		{
			packet_size += i_sprintf_ret;
			status = CMD_SUCCESS;
		}
		else
			status = CMD_FAILURE;
		break;
	}

	//calculate the checksums
	CalculateChecksums(cmdSuccess, packet_size);

	//send out the packet
	bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)cmdSuccess, (packet_size + CCSDS_HEADER_SIZE + CHECKSUM_SIZE));
	if(bytes_sent == (packet_size + CCSDS_HEADER_SIZE + CHECKSUM_SIZE))
		status = CMD_SUCCESS;
	else
		status = CMD_FAILURE;

	free(cmdSuccess);
	return status;
}

/**
 * Report the FAILURE packet for a function which was received, but failed
 *
 * @param Uart_PS	Pointer to the instance of the UART which will
 * 					transmit the packet to the spacecraft.
 *
 * @return	CMD_SUCCESS or CMD_FAILURE depending on if we sent out
 * 			the correct number of bytes with the packet.
 *
 * TODO: Model this function after the report success function, once it's checked out
 * 		I want this to be able to tell the user where they are, ie. they are in the
 * 		daq loop or some where else, which is why their function is not succeeding.
 */
int reportFailure(XUartPs Uart_PS)
{
	int status = 0;
	int bytes_sent = 0;
	int i_sprintf_ret = 0;
	unsigned char *cmdFailure = calloc(100, sizeof(unsigned char));
	PutCCSDSHeader(cmdFailure, GetLastCommandSize(), APID_CMD_FAIL);

	//fill the data bytes
	//print the command information after the CCSDS header
	i_sprintf_ret = snprintf((char *)cmdFailure + 11, 100, "%s\n", GetLastCommand());
	//check to make sure that the sizes match and we are reporting what we think we are
	if(i_sprintf_ret == GetLastCommandSize())
		status = CMD_SUCCESS;
	else
		status = CMD_FAILURE;
	//calculate the checksums
	CalculateChecksums(cmdFailure, i_sprintf_ret);
	//send out the packet
	bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)cmdFailure, (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE));
	if(bytes_sent == (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE))
		status = CMD_SUCCESS;
	else
		status = CMD_FAILURE;

	free(cmdFailure);
	return status;
}

/* Function to calculate all four checksums for CCSDS packets */
//This function calculates the Simple, Fletcher, and BCT checksums
// by looping over the bytes within the packet after the sync marker.
//
// @param	packet_array	This is a pointer to the CCSDS packet which
//							needs to have its checksums calculated.
// @param	length			The length of the packet data bytes.
// Note: the length should not account for the CCSDS Header, just give the length of the data bytes.
//
// @return	(int) returns the value assigned when the command was scanned
void CalculateChecksums(unsigned char * packet_array, int length)
{
	//this function will calculate the simple, Fletcher, and CCSDS checksums for any packet going out
	int packet_size = 0;
	int iterator = 0;
	int rmd_checksum_simple = 0;
	int rmd_checksum_Fletch = 0;
	unsigned short bct_checksum = 0;

	//put the length of the packet back together from the header bytes 8, 9
	packet_size = (packet_array[8] << 8) + packet_array[9];

	//loop over all the bytes in the packet and create checksums
	while(iterator < (packet_size - SYNC_MARKER_SIZE - CHECKSUM_SIZE))
	{
		rmd_checksum_simple = (rmd_checksum_simple + packet_array[SYNC_MARKER_SIZE + iterator]) % 255;
		rmd_checksum_Fletch = (rmd_checksum_Fletch + rmd_checksum_simple) % 255;
		bct_checksum += packet_array[SYNC_MARKER_SIZE + iterator];
		iterator++;
	}

	//write the checksums into the packet
	packet_array[CCSDS_HEADER_SIZE + length] = rmd_checksum_simple;
	packet_array[CCSDS_HEADER_SIZE + length + 1] = rmd_checksum_Fletch;
	packet_array[CCSDS_HEADER_SIZE + length + 2] = bct_checksum >> 8;
	packet_array[CCSDS_HEADER_SIZE + length + 3] = bct_checksum;

    return;
}

