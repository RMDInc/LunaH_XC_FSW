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
static int modu_board_temp = 25;
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
//			check_temp_sensor++;
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
		PutCCSDSHeader(report_buff, TEMP_PACKET_LENGTH, APID_TEMP);
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
		report_buff[35] = NEWLINE_CHAR_CODE;

		PutCCSDSHeader(report_buff, SOH_PACKET_LENGTH, APID_SOH);
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
			i_sprintf_ret = snprintf((char *)(&cmdSuccess[11 + i_sprintf_ret]), 100, "%s", GetFileName( DATA_TYPE_EVTS ));
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
	PutCCSDSHeader(cmdSuccess, packet_size + CHECKSUM_SIZE, APID_CMD_SUCC);
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

	PutCCSDSHeader(cmdFailure, GetLastCommandSize() + CHECKSUM_SIZE, APID_CMD_FAIL);
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


//Transfer options:
// 0 = data product file
// 1 = Log File
// 2 = Config file
/*
 * Transfers any one file that is on the SD card. Will return command FAILURE if the file does not exist.
 *
 * @param	Uart_PS	The instance of the UART so we can push the packet to the bus
 * @param 	(int)id_num 	The ID number for the folder the user wants to access
 * @param	(int)run_num	The Run number for the folder the user wants to access
 * @param	(int)file_type	The macro for the type of file to TX back, see lunah_defines.h for the codes
 * 							 There are 9 file types:
 * 							 DATA_TYPE_EVT, DATA_TYPE_CPS, DATA_TYPE_WAV,
 * 							 DATA_TYPE_2DH_1, DATA_TYPE_2DH_2, DATA_TYPE_2DH_3, DATA_TYPE_2DH_4,
 * 							 DATA_TYPE_LOG, DATA_TYPE_CFG,
 *
 * @param	(int)set_num_low	The set number to TX
 * @param	(int)set_num_hi		To TX a group of EVTs files, set this value higher than the first set number.
 *
 * NOTES: For this function, the file type is the important parameter because it tells the function how to
 * 			interpret the parameters which are given.
 * 		: For the Log and configuration files, parameters other than file_type should be written as 0's.
 * 		: For CPS, WAV, and 2DH files, the set numbers should be 0's.
 * 		: For EVT, the set numbers give a way to selectively transfer one or more set files at a time. If the
 * 			set_num_high value is 0, then just one set file will be TX'd. Otherwise, each set file from set low
 * 			to set high will be sent. There will be a checks on the user input, but no SOH in between the files.
 */
int TransferSDFile( XUartPs Uart_PS, int file_type, int id_num, int run_num, int set_num_low, int set_num_hi )
{
	int status = 0;	//0=good, 1=file DNE, 2+=other problem
	int sent = 0;
	int bytes_sent = 0;
	int total_sent = 0;
	int file_TX_size = 0;
	unsigned int bytes_written;
	unsigned int bytes_read = 0;
	unsigned int sizeof_tx_buffer = 0;
	char *file_to_TX;
	char *ptr_file_TX_filename;
	char empty_buff[20] = "";	//to keep from dereferencing an unassigned char pointer
	char log_file[] = "MNSCMDLOG.txt";
	char config_file[] = "MNSCONF.bin";
	char file_TX_folder[100] = "";
	char file_TX_filename[100] = "";
	char file_TX_path[100] = "";
	unsigned char tx_buffer[16392] = "";	//can transfer 16384/(4*8) = 512 evts/buff + 8 bytes for the "header"
	sizeof_tx_buffer = sizeof(tx_buffer);
	FIL TXFile;			//file object
	FILINFO fno;		//file info structure
	FRESULT f_res = FR_OK;	//SD card status variable type

	switch(file_type)
	{
	case DATA_TYPE_EVTS:
		//set up event-by-event data structures, packets

		break;
	case DATA_TYPE_WAV:
		//Waveform packets
		//where are these files stored? I think they get their own folders

		break;
	case DATA_TYPE_CPS:
		//cps file

		break;
	case DATA_TYPE_2DH_1:
		//2DH file for pmt 1

		break;
	case DATA_TYPE_2DH_2:
		//2DH file for pmt 2

		break;
	case DATA_TYPE_2DH_3:
		//2DH file for pmt 3

		break;
	case DATA_TYPE_2DH_4:
		//2DH file for pmt 4

		break;
	case DATA_TYPE_LOG:
		//log file

		break;
	case DATA_TYPE_CFG:
		//configuration file

		break;
	default:
		//if the file requested is not from the list of files, send back CMD_FAILURE
		status = 1;
		break;
	}

	//find the folder/file that was requested
	//create the folder name and file name to look for
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
		bytes_written = snprintf(file_TX_folder, 100, "0:/I%04d_R%04d", id_num, run_num);
		if(bytes_written == 0 || bytes_written != ROOT_DIR_NAME_SIZE + FOLDER_NAME_SIZE)
			status = 1;
		bytes_written = snprintf(file_TX_filename, 100, "evt_S%04d.bin", set_num_low);
		if(bytes_written == 0)
			status = 1;
		ptr_file_TX_filename = file_TX_filename;
	}
	//write the total file path
	bytes_written = snprintf(file_TX_path, 100, "%s/%s", file_TX_folder, ptr_file_TX_filename);
	if(bytes_written == 0)
		status = 1;

	//check that the folder/file we just wrote exists in the file system
	f_res = f_stat(file_TX_path, &fno);
	if(f_res == FR_NO_FILE)
	{
		//couldn't find the folder
		status = 1;	//folder DNE
	}
	//can just do an open on the dir:/folder/file.bin if we want, that way we don't have to use chdir or anything
	if(status == 0)
	{
		f_res = f_open(&TXFile, file_TX_path, FA_READ | FA_OPEN_ALWAYS);	//the files exists, so just open it
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
		if(file_type != DATA_TYPE_LOG && file_type != DATA_TYPE_CFG)	//read in the data header from data product files
		{
			DATA_FILE_HEADER_TYPE data_file_header;
			f_res = f_read(&TXFile, &data_file_header, sizeof(data_file_header), &bytes_read);
			if(f_res != FR_OK || bytes_read != sizeof(data_file_header.configBuff))
				status = 2;
			//forward seek to pass through the rest of the header, which should be 16384 bytes total
//			f_res = f_lseek(&TXFile, DP_HEADER_SIZE);	//bring this in once that change is made
		}
		else if(file_type == DATA_TYPE_CFG)	//the config file is just one config header
		{
			CONFIG_STRUCT_TYPE config_file_header;
			f_res = f_read(&TXFile, &config_file_header, sizeof(config_file_header), &bytes_read);
			if(f_res != FR_OK || bytes_read != sizeof(config_file_header))
				status = 2;
		}
		//no header information in the log file
	}
	//compile the CCSDS header
	if(status == 0)
	{

	}

	//compile the RMD data header (different based on file type)

	//read in data from the file to fill a packet

	//if packet isn't full, pad with 0's (pattern?)

	//send the file

	//loop back to //find if there are multiple files to send (EVT)

	//check return values/status variable

	//check for user interaction (break, too many bytes)

	tx_buffer[0] = (unsigned char)SYNC_MARKER;
	tx_buffer[1] = (unsigned char)(SYNC_MARKER >> 8);
	tx_buffer[2] = (unsigned char)(SYNC_MARKER >> 16);
	tx_buffer[3] = (unsigned char)(SYNC_MARKER >> 24);
	tx_buffer[4] = 0xAA;
	tx_buffer[5] = 0xFF;
	file_to_TX = empty_buff;	//make sure that something is here, at least
//	switch(file_to_access)
//	{
//	case TX_CMD:
//		file_to_TX = GetFileToAccess();	//gets the file name from ReadCommandType
//		break;
//	case TXLOG_CMD:
//		file_to_TX = log_file;
//		break;
//	case CONF_CMD:
//		file_to_TX = config_file;
//		break;
//	default:
//		file_to_TX = empty_buff;
//		break;
//	}

	f_res = f_open(&TXFile, file_to_TX, FA_READ|FA_OPEN_EXISTING);
	if(f_res != FR_OK)
		xil_printf("1 open file fail TX\n");
	if(f_res == FR_OK)
	{
		f_res = f_lseek(&TXFile, 0);
		if(f_res != FR_OK)
			xil_printf("2 lseek fail TX\n");

		sleep(1);
		xil_printf("\n\nFile size: %d\n", file_size(&TXFile));
		sleep(1);
		while(f_res == FR_OK)
		{
			f_res = f_read(&TXFile, &(tx_buffer[8]), sizeof_tx_buffer-8, &bytes_read);
			if(f_res != FR_OK)
				break;
			bytes_read += 8;
			tx_buffer[6] = (unsigned char)(bytes_read >> 8);
			tx_buffer[7] = (unsigned char)(bytes_read);

			sent = 0;
			bytes_sent = 0;
			while(sent < bytes_read)
			{
				bytes_sent = XUartPs_Send(&Uart_PS, &(tx_buffer[sent]), bytes_read - sent);
				sent += bytes_sent;
				total_sent += bytes_sent;
			}
			if(bytes_read < 16384)
				break;
		}
		if(f_res != FR_OK)
			status = 2;
		else
			status = 0;
	}
	else if(f_res == FR_NO_FILE)
	{
		//file does not exist
		status = 1;
	}
	else
	{
		//there was a problem
		status = 2;
	}

	f_close(&TXFile);

	sleep(1);
	xil_printf("\n\nFile size: %d\n", total_sent);
	sleep(1);

	return status;
}
