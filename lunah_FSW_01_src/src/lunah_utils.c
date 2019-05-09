/*
 * lunah_utils.c
 *
 *  Created on: Jun 22, 2018
 *      Author: IRDLAB
 */

#include "lunah_utils.h"

static XTime LocalTime = 0;
static XTime TempTime = 0;
static XTime LocalTimeStart;
static XTime LocalTimeCurrent = 0;

//may still need these if we want to 'get' the temp at some point
//also, need to verify that we are getting the correct temp
static int analog_board_temp = 25;
static int digital_board_temp = 1;
static int modu_board_temp = 25;
static int iNeutronTotal = 0;
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

int PutNeutronTotal(int total)
{
	iNeutronTotal = total;
	return iNeutronTotal;
}

int IncNeutronTotal(int increment)
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
//  int iNeutronTotal;

	XTime_GetTime(&LocalTimeCurrent);
	if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (LocalTime +  1))
	{
//		iNeutronTotal = GetNeutronTotal();
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
	unsigned int local_time_holder = 0;

	i2c_Send_Buffer[0] = 0x0;
	i2c_Send_Buffer[1] = 0x0;
	int IIC_SLAVE_ADDR2 = 0x4B;	//Temp sensor on digital board
//	int IIC_SLAVE_ADDR3 = 0x48;	//Temp sensor on the analog board
//	int IIC_SLAVE_ADDR5 = 0x4A;	//Extra Temp Sensor Board, on module near thermistor on TEC

	switch(check_temp_sensor){
	case 0:	//analog board
		XTime_GetTime(&LocalTimeCurrent);
		if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (TempTime + 2))
		{
//			TempTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND; //temp time is reset
			check_temp_sensor++;
//			IicPsMasterSend(Iic, IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, &IIC_SLAVE_ADDR3);
//			IicPsMasterRecieve(Iic, i2c_Recv_Buffer, &IIC_SLAVE_ADDR3);
//			a = i2c_Recv_Buffer[0]<< 5;
//			b = a | i2c_Recv_Buffer[1] >> 3;
//			if(i2c_Recv_Buffer[0] >= 128)
//			{
//				b = (b - 8192) / 16;
//			}
//			else
//			{
//				b = b / 16;
//			}
			b = 23;
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
			modu_board_temp += 1;
		}
		break;
	default:
		status = CMD_FAILURE;
		break;
	}

	//to replace the printf statement, we need to sort the integer temps into the array so they have fixed widths
	// and since we're already using a char array, we'll sort the ints into chars
	//do this for anlg, digi, and modu, then take that out of the cases below
	//will still need to do this in the getstat case b/c have to include the n total and time
	report_buff[11] = (unsigned char)(analog_board_temp >> 24);
	report_buff[12] = (unsigned char)(analog_board_temp >> 16);
	report_buff[13] = (unsigned char)(analog_board_temp >> 8);
	report_buff[14] = (unsigned char)(analog_board_temp);
	report_buff[15] = TAB_CHAR_CODE;
	report_buff[16] = (unsigned char)(digital_board_temp >> 24);
	report_buff[17] = (unsigned char)(digital_board_temp >> 16);
	report_buff[18] = (unsigned char)(digital_board_temp >> 8);
	report_buff[19] = (unsigned char)(digital_board_temp);
	report_buff[20] = TAB_CHAR_CODE;
	report_buff[21] = (unsigned char)(modu_board_temp >> 24);
	report_buff[22] = (unsigned char)(modu_board_temp >> 16);
	report_buff[23] = (unsigned char)(modu_board_temp >> 8);
	report_buff[24] = (unsigned char)(modu_board_temp);
	report_buff[25] = TAB_CHAR_CODE;


	switch(packet_type)
	{
	case READ_TMP_CMD:
		PutCCSDSHeader(report_buff, APID_TEMP, GF_UNSEG_PACKET, 1, TEMP_PACKET_LENGTH);
		CalculateChecksums(report_buff);

		bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, (TEMP_PACKET_LENGTH + CCSDS_HEADER_FULL));
		if(bytes_sent == (TEMP_PACKET_LENGTH + CCSDS_HEADER_FULL))
			status = CMD_SUCCESS;
		else
			status = CMD_FAILURE;
		break;
	case GETSTAT_CMD:
		report_buff[26] = (unsigned char)(i_neutron_total >> 24);
		report_buff[27] = (unsigned char)(i_neutron_total >> 16);
		report_buff[28] = (unsigned char)(i_neutron_total >> 8);
		report_buff[29] = (unsigned char)(i_neutron_total);
		report_buff[30] = TAB_CHAR_CODE;
		local_time_holder = (unsigned int)local_time;
		report_buff[31] = (unsigned char)(local_time_holder >> 24);
		report_buff[32] = (unsigned char)(local_time_holder >> 16);
		report_buff[33] = (unsigned char)(local_time_holder >> 8);
		report_buff[34] = (unsigned char)(local_time_holder);
		report_buff[35] = TAB_CHAR_CODE;
		report_buff[36] = MODE_STANDBY;		//this currently only prints that the detector is in standby mode //GJS 5/9/2019
		report_buff[37] = NEWLINE_CHAR_CODE;

		PutCCSDSHeader(report_buff, APID_SOH, GF_UNSEG_PACKET, 1, SOH_PACKET_LENGTH);
		CalculateChecksums(report_buff);

		bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, (SOH_PACKET_LENGTH + CCSDS_HEADER_FULL));
		if(bytes_sent == (SOH_PACKET_LENGTH + CCSDS_HEADER_FULL))
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

/*
 * Put the appropriate CCSDS header values into the output packet.
 *
 * @param SOH_buff	Pointer to the packet buffer
 *
 * @param length	The length of the packet is equal to the number of bytes in the secondary CCSDS header
 * 					plus the payload data bytes plus the checksums minus one.
 * 					Len = 1 + N + 4 - 1
 *
 * @return	CMD_SUCCESS or CMD_FAILURE depending on if we sent out
 * 			the correct number of bytes with the packet.
 *
 */
void PutCCSDSHeader(unsigned char * SOH_buff, int packet_type, int group_flags, int sequence_count, int length)
{
	//get the values for the CCSDS header
	SOH_buff[0] = 0x35;
	SOH_buff[1] = 0x2E;
	SOH_buff[2] = 0xF8;
	SOH_buff[3] = 0x53;
	if(MNS_DETECTOR_NUM == 0)
		SOH_buff[4] = 0x0A; //identify detector 0 or 1
	else
		SOH_buff[4] = 0x0B;
	//use the input to determine what APID to fill here
	switch(packet_type)
	{
	case APID_CMD_SUCC:
		SOH_buff[5] = 0x00;	//APID for Command Success
		break;
	case APID_CMD_FAIL:
		SOH_buff[5] = 0x11;	//APID for Command Failure
		break;
	case APID_SOH:
		SOH_buff[5] = 0x22;	//APID for SOH
		break;
	case APID_LS_FILES:
		SOH_buff[5] = 0x33;	//APID for LS Files
		break;
	case APID_TEMP:
		SOH_buff[5] = 0x44;	//APID for Temperature
		break;
	case APID_MNS_CPS:
		SOH_buff[5] = 0x55;	//APID for Counts per second
		break;
	case APID_MNS_WAV:
		SOH_buff[5] = 0x66;	//APID for Waveforms
		break;
	case APID_MNS_EVT:
		SOH_buff[5] = 0x77;	//APID for Event-by-Event
		break;
	case APID_MNS_2DH:
		SOH_buff[5] = 0x88;	//APID for 2D Histogram
		break;
	case APID_LOG_FILE:
		SOH_buff[5] = 0x99;	//APID for Log
		break;
	case APID_CONFIG:
		SOH_buff[5] = 0xAA;	//APID for Configuration
		break;
	case DATA_TYPE_2DH_2:
		SOH_buff[5] = 0x88;	//APID for 2D Histogram
		break;
	case DATA_TYPE_2DH_3:
		SOH_buff[5] = 0x88;	//APID for 2D Histogram
		break;
	case DATA_TYPE_2DH_4:
		SOH_buff[5] = 0x88;	//APID for 2D Histogram
		break;
	default:
		SOH_buff[5] = 0x22; //default to SOH just in case?
		break;
	}

	SOH_buff[6] = (unsigned char)(sequence_count >> 8);
	SOH_buff[6] &= 0x3F;
	SOH_buff[6] |= (unsigned char)(group_flags << 6);
	SOH_buff[7] = (unsigned char)(sequence_count);
	SOH_buff[8] = (length & 0xFF00) >> 8;
	SOH_buff[9] = length & 0xFF;

	SOH_buff[10] = 0x00;	//TODO: actually set the conditions to report a reset request

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
	unsigned char cmdSuccess[100] = "";

	//fill the data bytes
	switch(report_filename)
	{
	case 1:
		//Enabled the switch to report the filename
		//should we check to see if the last command was DAQ/WF? No for now
		i_sprintf_ret = snprintf((char *)(&cmdSuccess[11]), 100, "%s\n", GetLastCommand());
		if(i_sprintf_ret == GetLastCommandSize())
		{
			packet_size += i_sprintf_ret;
			//now we want to add in the new filename that is going to be used
			//TODO: report the folder instead of the filename (the run number is in the folder name now)
			i_sprintf_ret = snprintf((char *)(&cmdSuccess[11 + i_sprintf_ret]), 100, "%s", GetFolderName());
			if(i_sprintf_ret == GetFolderNameSize())
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
		i_sprintf_ret = snprintf((char *)(&cmdSuccess[11]), 100, "%s\n", GetLastCommand());
		if(i_sprintf_ret == GetLastCommandSize())
		{
			packet_size += i_sprintf_ret;
			status = CMD_SUCCESS;
		}
		else
			status = CMD_FAILURE;
		break;
	}

	//I should look at using a regular char buffer rather than using calloc() and free() //changed 3/14/19
	//get last command gets the size of the string minus the newline
	//we want to add 1 (secondary header) and add 4 (checksums) minus 1
	PutCCSDSHeader(cmdSuccess, APID_CMD_SUCC, GF_UNSEG_PACKET, 1, packet_size + CHECKSUM_SIZE);
	CalculateChecksums(cmdSuccess);

	bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)cmdSuccess, (CCSDS_HEADER_FULL + packet_size + CHECKSUM_SIZE));
	if(bytes_sent == (CCSDS_HEADER_FULL + packet_size + CHECKSUM_SIZE))
		status = CMD_SUCCESS;
	else
		status = CMD_FAILURE;

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
	unsigned char cmdFailure[100] = "";

	i_sprintf_ret = snprintf((char *)(&cmdFailure[11]), 100, "%s\n", GetLastCommand());
	if(i_sprintf_ret == GetLastCommandSize())
		status = CMD_SUCCESS;
	else
		status = CMD_FAILURE;

	PutCCSDSHeader(cmdFailure, APID_CMD_FAIL, GF_UNSEG_PACKET, 1, GetLastCommandSize() + CHECKSUM_SIZE);
	CalculateChecksums(cmdFailure);

	bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)cmdFailure, (CCSDS_HEADER_FULL + i_sprintf_ret + CHECKSUM_SIZE));
	if(bytes_sent == (CCSDS_HEADER_FULL + i_sprintf_ret + CHECKSUM_SIZE))
		status = CMD_SUCCESS;
	else
		status = CMD_FAILURE;

	return status;
}

/* Function to calculate all four checksums for CCSDS packets
 * This function calculates the Simple, Fletcher, and BCT checksums
 *  by looping over the bytes within the packet after the sync marker.
 *
 *  @param	packet_array	This is a pointer to the CCSDS packet which
 *    							needs to have its checksums calculated.
 *	@param	length			The packet length
 *
 *	@return	(int) returns the value assigned when the command was scanned
 */
void CalculateChecksums(unsigned char * packet_array)
{
	//this function will calculate the simple, Fletcher, and CCSDS checksums for any packet going out
	int packet_size = 0;
	int total_packet_size = 0;
	int iterator = 0;
	int rmd_checksum_simple = 0;
	int rmd_checksum_Fletch = 0;
	unsigned short bct_checksum = 0;

	packet_size = (packet_array[8] << 8) + packet_array[9];	//from the packet, includes payload data plus checksums
	total_packet_size = packet_size + CCSDS_HEADER_FULL;	//includes both primary and secondary CCSDS headers

	//create the RMD checksums
	while(iterator <= (packet_size - CHECKSUM_SIZE))
	{
		rmd_checksum_simple = (rmd_checksum_simple + packet_array[CCSDS_HEADER_PRIM + iterator]) % 255;
		rmd_checksum_Fletch = (rmd_checksum_Fletch + rmd_checksum_simple) % 255;
		iterator++;
	}

	packet_array[total_packet_size - CHECKSUM_SIZE] = rmd_checksum_simple;
	packet_array[total_packet_size - CHECKSUM_SIZE + 1] = rmd_checksum_Fletch;

	//calculate the BCT checksum
	iterator = 0;
	while(iterator < (packet_size - RMD_CHECKSUM_SIZE + CCSDS_HEADER_DATA))
	{
		bct_checksum += packet_array[SYNC_MARKER_SIZE + iterator];
		iterator++;
	}

	packet_array[total_packet_size - CHECKSUM_SIZE + 2] = bct_checksum >> 8;
	packet_array[total_packet_size - CHECKSUM_SIZE + 3] = bct_checksum;

    return;
}

/*
 * Transfers any one file that is on the SD card. Will return command FAILURE if the file does not exist.
 *
 * @param	(XUartPS)The instance of the UART so we can push packets to the bus
 * @param	(char *)pointer to the receive buffer to check for a BREAK
 * @param	(int)file_type	The macro for the type of file to TX back, see lunah_defines.h for the codes
 * 							 There are 9 file types:
 * 							 DATA_TYPE_EVT, DATA_TYPE_CPS, DATA_TYPE_WAV,
 * 							 DATA_TYPE_2DH_1, DATA_TYPE_2DH_2, DATA_TYPE_2DH_3, DATA_TYPE_2DH_4,
 * 							 DATA_TYPE_LOG, DATA_TYPE_CFG
 * @param 	(int)id_num 	The ID number for the folder the user wants to access
 * @param	(int)run_num	The Run number for the folder the user wants to access *
 * @param	(int)set_num_low	The set number to TX, if multiple files are requested by the user, the
 * 								 calling function will call this function multiple times with a different
 * 								 set number each time.
 *
 * @return	(int) returns the status of the transfer, 0 = good, 1 = file DNE, 2+ = other problems
 *
 * NOTES: For this function, the file type is the important parameter because it tells the function how to
 * 			interpret the parameters which are given.
 * 		: For the Log and configuration files, parameters other than file_type should be written as 0's.
 * 		: For CPS, WAV, and 2DH files, the set numbers should be 0's.
 * 		: For EVT, the set numbers give a way to selectively transfer one or more set files at a time. If the
 * 			set_num_high value is 0, then just one set file will be TX'd. Otherwise, each set file from set low
 * 			to set high will be sent. There will be a checks on the user input, but no SOH in between the files.
 */
int TransferSDFile( XUartPs Uart_PS, char * RecvBuffer, int file_type, int id_num, int run_num, int set_num )
{
	int status = 0;			//0=good, 1=file DNE, 2+=other problem
	int poll_val = 0;		//local polling status variable
	int sent = 0;			//bytes sent by UART
	int bytes_sent = 0;
	unsigned short s_holder = 0;
	float f_holder = 0;
	int file_TX_size = 0;				//tracks number of bytes left to TX in the file TOTAL
	int file_TX_packet_size = 0;		//number of bytes to send //number of bytes in a packet total
	int file_TX_data_bytes_size = 0;	//size of the data bytes for that type of data product packet
	int file_TX_packet_header_size = 0;	//size of the packet header bytes minus the CCSDS primary header (10 bytes)
	int file_TX_add_padding = 0;		//flag to add padding bytes to an outgoing packet
	int file_TX_group_flags = 0;
	int file_TX_sequence_count = 0;
	int file_TX_apid = 0;
	int m_loop_var = 1;					//0 = false; 1 = true
	int bytes_to_read = 0;				//number of bytes to read from data file to put into packet data bytes
	unsigned int bytes_written;
	unsigned int bytes_read = 0;
	char *ptr_file_TX_filename;
	char log_file[] = "MNSCMDLOG.txt";
	char config_file[] = "MNSCONF.bin";
	char file_TX_folder[100] = "";
	char file_TX_filename[100] = "";
	char file_TX_path[100] = "";
	unsigned char packet_array[2040] = "";
	DATA_FILE_HEADER_TYPE data_file_header = {};
	DATA_FILE_SECONDARY_HEADER_TYPE data_file_2ndy_header = {};
	CONFIG_STRUCT_TYPE config_file_header = {};
	FIL TXFile;				//file object
	FILINFO fno;			//file info structure
	FRESULT f_res = FR_OK;	//SD card status variable type

	//find the folder/file that was requested
	if(file_type == DATA_TYPE_LOG)
	{
		//just on the root directory
		bytes_written = snprintf(file_TX_folder, 100, "0:");
		if(bytes_written == 0 || bytes_written != ROOT_DIR_NAME_SIZE)
			status = 1;
		ptr_file_TX_filename = log_file;
	}
	else if(file_type == DATA_TYPE_CFG)
	{
		//just on the root directory
		bytes_written = snprintf(file_TX_folder, 100, "0:");
		if(bytes_written == 0 || bytes_written != ROOT_DIR_NAME_SIZE)
			status = 1;
		ptr_file_TX_filename = config_file;
	}
	else
	{
		//construct the folder
		bytes_written = snprintf(file_TX_folder, 100, "0:/I%04d_R%04d", id_num, run_num);
		if(bytes_written == 0 || bytes_written != ROOT_DIR_NAME_SIZE + FOLDER_NAME_SIZE)
			status = 1;
		//construct the file name
		if(file_type == DATA_TYPE_EVT)
		{
			bytes_written = snprintf(file_TX_filename, 100, "evt_S%04d.bin", set_num);
			if(bytes_written == 0)
				status = 1;
		}
		else if(file_type == DATA_TYPE_WAV)
		{
			bytes_written = snprintf(file_TX_filename, 100, "wav_S%04d.bin", set_num);
			if(bytes_written == 0)
				status = 1;
		}
		else if(file_type == DATA_TYPE_CPS)
		{
			bytes_written = snprintf(file_TX_filename, 100, "cps_S%04d.bin", set_num);
			if(bytes_written == 0)
				status = 1;
		}
		else if(file_type == DATA_TYPE_2DH_1)
		{
			bytes_written = snprintf(file_TX_filename, 100, "2d1_S%04d.bin", set_num);
			if(bytes_written == 0)
				status = 1;
		}
		else if(file_type == DATA_TYPE_2DH_2)
		{
			bytes_written = snprintf(file_TX_filename, 100, "2d2_S%04d.bin", set_num);
			if(bytes_written == 0)
				status = 1;
		}
		else if(file_type == DATA_TYPE_2DH_3)
		{
			bytes_written = snprintf(file_TX_filename, 100, "2d3_S%04d.bin", set_num);
			if(bytes_written == 0)
				status = 1;
		}
		else if(file_type == DATA_TYPE_2DH_4)
		{
			bytes_written = snprintf(file_TX_filename, 100, "2d4_S%04d.bin", set_num);
			if(bytes_written == 0)
				status = 1;
		}

		ptr_file_TX_filename = file_TX_filename;
	}

	//write the total file path
	bytes_written = snprintf(file_TX_path, 100, "%s/%s", file_TX_folder, ptr_file_TX_filename);
	if(bytes_written == 0)
		status = 1;

	//check that the folder/file we just wrote exists in the file system
	//check first so that we don't just open a blank new file; there are no protections for that
	f_res = f_stat(file_TX_path, &fno);
	if(f_res == FR_NO_FILE)
	{
		//couldn't find the folder
		status = 1;	//folder DNE
	}

	if(status == 0)
	{
		//can just do an open on the dir:/folder/file.bin if we want, that way we don't have to use chdir or anything
		f_res = f_open(&TXFile, file_TX_path, FA_READ);	//the files exists, so just open it //only do fa-read so that we don't open a new file
		if(f_res != FR_OK)
		{
			if(f_res == FR_NO_PATH)
				status = 1;
			else
				status = 2;
		}
	}
	//read in important information (file size, header, first event, real time, etc.)
	if(status == 0)
	{
		file_TX_size = file_size(&TXFile);
		if(file_type != DATA_TYPE_LOG && file_type != DATA_TYPE_CFG)	//EVT, CPS, 2DH, WAV files
		{
			f_res = f_read(&TXFile, &data_file_header, sizeof(data_file_header), &bytes_read);	//read in 188 bytes, up to the real time
			if(f_res != FR_OK || bytes_read != sizeof(data_file_header))
				status = 2;
			else
				file_TX_size -= bytes_read;

			if(file_type == DATA_TYPE_EVT || file_type == DATA_TYPE_WAV || file_type == DATA_TYPE_CPS) //2DH files don't have this
			{
				f_res = f_read(&TXFile, &data_file_2ndy_header, sizeof(data_file_2ndy_header), &bytes_read);	//read in the real times
				if(f_res != FR_OK)
				{
					//TODO: can do a check that the eventID bytes are correct here so we know that it's a good read?
					// could use this as a verifcation, but maybe it's too much
					status = 2;
				}
				else
					file_TX_size -= bytes_read;
			}
			if(file_type == DATA_TYPE_EVT)
			{
				f_res = f_lseek(&TXFile, DP_HEADER_SIZE);
				if(f_res != FR_OK)
					status = 2;
				else
					file_TX_size -= (DP_HEADER_SIZE - sizeof(data_file_header) - sizeof(data_file_2ndy_header));
			}
		}
		else if(file_type == DATA_TYPE_CFG)	//the config file is just one config header
		{
			f_res = f_read(&TXFile, &config_file_header, sizeof(config_file_header), &bytes_read);
			if(f_res != FR_OK || bytes_read != sizeof(config_file_header))
				status = 2;
			else
				file_TX_size -= bytes_read;
		}
		//no header information in the log file //need to assign the
	}

	//compile the RMD data header (different based on file type)
	if(status == 0)
	{
		//for EVT file type
		//fill in the RMD header	//these are shared header values for CPS, 2DH, EVT, WAV, CFG //only LOG doesn't have this
		f_holder = data_file_header.configBuff.ScaleFactorEnergy_1_1;	memcpy(&(packet_array[11]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.ScaleFactorEnergy_1_2;	memcpy(&(packet_array[15]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.ScaleFactorPSD_1_1;		memcpy(&(packet_array[19]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.ScaleFactorPSD_1_2;		memcpy(&(packet_array[23]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.OffsetEnergy_1_1;		memcpy(&(packet_array[27]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.OffsetEnergy_1_2;		memcpy(&(packet_array[31]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.OffsetPSD_1_1;			memcpy(&(packet_array[35]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.OffsetPSD_1_2;			memcpy(&(packet_array[39]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.ECalSlope;				memcpy(&(packet_array[43]), &f_holder, sizeof(float));
		f_holder = data_file_header.configBuff.ECalIntercept;			memcpy(&(packet_array[47]), &f_holder, sizeof(float));
		s_holder = (unsigned short)data_file_header.configBuff.TriggerThreshold;	memcpy(&(packet_array[51]), &s_holder, sizeof(s_holder));
		s_holder = (unsigned short)data_file_header.configBuff.IntegrationBaseline;	memcpy(&(packet_array[53]), &s_holder, sizeof(s_holder));
		s_holder = (unsigned short)data_file_header.configBuff.IntegrationShort;	memcpy(&(packet_array[55]), &s_holder, sizeof(s_holder));
		s_holder = (unsigned short)data_file_header.configBuff.IntegrationLong;		memcpy(&(packet_array[57]), &s_holder, sizeof(s_holder));
		s_holder = (unsigned short)data_file_header.configBuff.IntegrationFull;		memcpy(&(packet_array[59]), &s_holder, sizeof(s_holder));
		s_holder = (unsigned short)data_file_header.configBuff.HighVoltageValue[0];	memcpy(&(packet_array[61]), &s_holder, sizeof(s_holder));
		s_holder = (unsigned short)data_file_header.configBuff.HighVoltageValue[1];	memcpy(&(packet_array[63]), &s_holder, sizeof(s_holder));
		s_holder = (unsigned short)data_file_header.configBuff.HighVoltageValue[2];	memcpy(&(packet_array[65]), &s_holder, sizeof(s_holder));
		s_holder = (unsigned short)data_file_header.configBuff.HighVoltageValue[3];	memcpy(&(packet_array[67]), &s_holder, sizeof(s_holder));

		if(file_type == DATA_TYPE_EVT || file_type == DATA_TYPE_WAV || file_type == DATA_TYPE_CPS)
		{
			memcpy(&(packet_array[69]), &data_file_2ndy_header.RealTime, sizeof(data_file_2ndy_header.RealTime));
			memcpy(&(packet_array[77]), &data_file_2ndy_header.FirstEventTime, sizeof(data_file_2ndy_header.FirstEventTime));
		}
	}
	if(status != 0)
		m_loop_var = 0;	//don't loop, just exit
	//loop here to compile the packets, all above stuff is necessary once and stays the same in each packet
	//This loop compiles the remaining parts of the packet and sends it
	// then repeats until there is no more data
	while(m_loop_var == 1)
	{
		//here is where I'll get data product specific
		switch(file_type)
		{
		case DATA_TYPE_EVT:
			//what do we need to specify to make things correct for one data product or another?
			file_TX_data_bytes_size = DATA_BYTES_EVT;
			file_TX_packet_size = PKT_SIZE_EVT;
			file_TX_packet_header_size = PKT_HEADER_EVT;
			file_TX_apid = 0x77;
			break;
		case DATA_TYPE_WAV:
			file_TX_data_bytes_size = DATA_BYTES_WAV;
			file_TX_packet_size = PKT_SIZE_WAV;
			file_TX_packet_header_size = PKT_HEADER_WAV;
			file_TX_apid = 0x66;
			break;
		case DATA_TYPE_CPS:
			file_TX_data_bytes_size = DATA_BYTES_CPS;
			file_TX_packet_size = PKT_SIZE_CPS;
			file_TX_packet_header_size = PKT_HEADER_CPS;
			file_TX_apid = 0x55;
			break;
		case DATA_TYPE_2DH_1:
			/* Falls through to case 2DH_2 */
		case DATA_TYPE_2DH_2:
			/* Falls through to case 2DH_3 */
		case DATA_TYPE_2DH_3:
			/* Falls through to case 2DH_4 */
		case DATA_TYPE_2DH_4:
			file_TX_data_bytes_size = DATA_BYTES_2DH;
			file_TX_packet_size = PKT_SIZE_2DH;
			file_TX_packet_header_size = PKT_HEADER_2DH;
			file_TX_apid = 0x88;
			break;
		case DATA_TYPE_LOG:
			//can't do these yet
			file_TX_data_bytes_size = DATA_BYTES_LOG;
			file_TX_packet_size = PKT_SIZE_LOG;
			file_TX_packet_header_size = PKT_HEADER_LOG;
			file_TX_apid = 0x99;
			break;
		case DATA_TYPE_CFG:
			file_TX_data_bytes_size = DATA_BYTES_CFG;
			file_TX_packet_size = PKT_SIZE_CFG;
			file_TX_packet_header_size = PKT_HEADER_CFG;
			file_TX_apid = 0xAA;
			break;
		default:
			//how did we get here if the file type is wrong?
			break;
		}

		if(file_TX_size > file_TX_data_bytes_size)
		{
			bytes_to_read = file_TX_data_bytes_size;
			file_TX_add_padding = 0;
			if(file_TX_sequence_count == 0)
				file_TX_group_flags = 1;	//first packet
			else
				file_TX_group_flags = 0;	//intermediate packet
		}
		else
		{
			bytes_to_read = file_TX_size;
			file_TX_add_padding = 1;
			if(file_TX_sequence_count == 0)
				file_TX_group_flags = 3;	//unsegmented packet
			else
				file_TX_group_flags = 2;	//last packet
		}
		//have to match up APIDs (real value) with the APID codes in lunah defines

		PutCCSDSHeader(packet_array, data_file_header.FileTypeAPID, file_TX_group_flags, file_TX_sequence_count, file_TX_packet_size);
		//read in the data bytes
		f_res = f_read(&TXFile, &(packet_array[CCSDS_HEADER_PRIM + file_TX_packet_header_size]), bytes_to_read, &bytes_read);
		if(f_res != FR_OK)
			status = 2;
		else
			file_TX_size -= bytes_to_read; //TODO: do we want to subtract this number or bytes_read the actual numbers of bytes read?
		//add padding bytes, if necessary
		if(file_TX_add_padding == 1)
		{
			memset(&(packet_array[CCSDS_HEADER_PRIM + file_TX_packet_header_size + bytes_read]), file_TX_apid, file_TX_packet_size - bytes_read);
			file_TX_add_padding = 0;	//reset
		}
		//calculate the checksums for the packet
		CalculateChecksums(packet_array);

		//send the packet
		sent = 0;
		bytes_sent = 0;
		file_TX_packet_size += CCSDS_HEADER_FULL;	//the full packet size in bytes
		while(sent < file_TX_packet_size)
		{
			bytes_sent = XUartPs_Send(&Uart_PS, &(packet_array[sent]), file_TX_packet_size - sent);
			sent += bytes_sent;
		}

		//check if there are multiple packets/files to send (EVT)
		switch(file_TX_group_flags)
		{
		case 0:	//intermediate packet
			/* Falls through to case 1 */
		case 1:	//first packet
			m_loop_var = 1;
			file_TX_sequence_count++;
			//erase the parts of the packet which are unique so they don't get put into the next packet
			memset(&(packet_array[6]), '\0', 2);	//reset group flags, sequence count
			memset(&(packet_array[10]), '\0', 1);	//reset secondary header (reset request bits)
			if(file_type == DATA_TYPE_EVT || file_type == DATA_TYPE_WAV || file_type == DATA_TYPE_CPS)
				memset(&(packet_array[81]), '\0', file_TX_packet_size - CCSDS_HEADER_PRIM - file_TX_packet_header_size);
			else if(file_type == DATA_TYPE_2DH_1 || file_type == DATA_TYPE_2DH_2 || file_type == DATA_TYPE_2DH_3 || file_type == DATA_TYPE_2DH_4 )
				memset(&(packet_array[85]), '\0', file_TX_packet_size - CCSDS_HEADER_PRIM - PKT_HEADER_2DH);
			else if(file_type == DATA_TYPE_LOG)
				memset(&(packet_array[11]), '\0', file_TX_packet_size- CCSDS_HEADER_PRIM);
			//no need to erase the config file
			break;
		case 2:	//current packet was last packet
			/* Falls through to case 3 */
		case 3:	//current packet was unsegmented
			m_loop_var = 0;
			break;
		default:
			//TODO: error check bad group flags, for now just be done, don't loop back
			m_loop_var = 0;
			status = 2;
			break;
		}

		//check for user interaction (break, too many bytes)
		poll_val = ReadCommandType(RecvBuffer, &Uart_PS);
		switch(poll_val)
		{
		case -1:
			//this is bad input or an error in input
			//should handle this separately from default
			break;
		case BREAK_CMD:
			m_loop_var = 0;	//done looping
			status = DAQ_BREAK;
			break;
		default:
			break;
		}

	}//END OF WHILE(m_loop_var == 1)

	f_close(&TXFile);

	return status;
}
