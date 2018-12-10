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

FILINFO CnfFno;
char cConfigFile[] = "1:/ConfigFile.cnf";
FIL ConfigFile;
int filptr_cConfigFile = 0;

CONFIG_STRUCT_TYPE ConfigBuff;
int ConfigSize = sizeof(ConfigBuff);

static XTime LocalTime = 0;
static XTime TempTime = 0;
static XTime LocalTimeStart;
static XTime LocalTimeCurrent = 0;

//may still need these if we want to 'get' the temp at some point
//also, need to verify that we are getting the correct temp
static int analog_board_temp = 1;
static int digital_board_temp = 1;
static float modu_board_temp = 25;
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
int CheckForSOH(XIicPs Iic, XUartPs Uart_PS)
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
int report_SOH(XIicPs Iic, XTime local_time, int i_neutron_total, XUartPs Uart_PS, int packet_type)
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

	//if temp has not been checked in 2s, add 0.5 degrees to the temp
	//if check_temp_sensor is 0, check analog board temp sensor, else check the next sensor
	// then increment check_temp_sensor and reset time to next check
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
 * Utility function to convert an int representing a command
 * into a string itself. Modularized away from reportSuccess()
 * and reportFailure().
 *
 * TODO: Maybe we can use ReadCommandType() in main.c, because
 * We already read the recvBuffer there, then convert it to an int...
 * Just to convert it back to a char[]. Instead, we can get the command
 * in main.c using an overridden ReadCommandType(), and return the command
 * as a char instead of int, bypassing this function.
 *
 * I have added a command to go and get the text and size of the
 * previously entered command. They are in the ReadCommandType.c file.
 */
int parseCommand(int menusel, char *cmdArr)
{
	int cmdLen;
	switch (menusel)
		{
		case 0:
			//DAQ
			strcpy(cmdArr, "DAQ");
			cmdLen = 3;
			break;
		case 1:
			//WF
			strcpy(cmdArr, "WF");
			cmdLen = 2;
			break;
		case 2:
			//TMP
			strcpy(cmdArr, "TMP");
			cmdLen = 3;
			break;
		case 3:
			//GETSTAT
			strcpy(cmdArr, "GETSTAT");
			cmdLen = 7;
			break;
		case 4:
			//DISABLE_ACT
			strcpy(cmdArr, "DISABLE_ACT");
			cmdLen = 11;
			break;
		case 5:
			//DISABLE_TEC
			strcpy(cmdArr, "DISABLE_TEC");
			cmdLen = 11;
			break;
		case 6:
			//ENABLE_TEC
			strcpy(cmdArr, "ENABLE_TEC");
			cmdLen = 10;
			break;
		case 7:
			//TX
			strcpy(cmdArr, "TX");
			cmdLen = 2;
			break;
		case 8:
			//DEL
			strcpy(cmdArr, "DEL");
			cmdLen = 3;
			break;
		case 9:
			//LS
			strcpy(cmdArr, "LS");
			cmdLen = 2;
			break;
		case 10:
			//TRG
			strcpy(cmdArr, "TRG");
			cmdLen = 3;
			break;
		case 11:
			//NGATES
			strcpy(cmdArr, "NGATES");
			cmdLen = 6;
			break;
		case 12:
			//HV
			strcpy(cmdArr, "HV");
			cmdLen = 2;
			break;
		case 13:
			//INT
			strcpy(cmdArr, "INT");
			cmdLen = 3;
			break;
		case 14:
			//ECAL
			strcpy(cmdArr, "ECAL");
			cmdLen = 4;
			break;
		case 15:
			//BREAK
			strcpy(cmdArr, "BREAK");
			cmdLen = 5;
			break;
		case 16:
			//START
			strcpy(cmdArr, "START");
			cmdLen = 5;
			break;
		case 17:
			//END
			strcpy(cmdArr, "END");
			cmdLen = 3;
			break;
		case 18:
			//END_TMP
			strcpy(cmdArr, "END_TMP");
			cmdLen = 7;
			break;
		case 19:
			//READ_TMP
			strcpy(cmdArr, "READ_TMP");
			cmdLen = 8;
			break;
		default:
			strcpy(cmdArr, "INVALID_CMD");
			cmdLen = 11;
			break;
		}

	return cmdLen;
}

/**
 * Report the SUCCESS packet for a function which was received and passed
 *
 * @param Uart_PS	Pointer to the instance of the UART which will
 * 					transmit the packet to the spacecraft.
 *
 * @return	CMD_SUCCESS or CMD_FAILURE depending on if we sent out
 * 			the correct number of bytes with the packet.
 *
 */
int reportSuccess(XUartPs Uart_PS)
{
	int status = 0;
	int bytes_sent = 0;
	int i_sprintf_ret = 0;
	unsigned char *cmdSuccess = malloc(100);
	PutCCSDSHeader(cmdSuccess, GetLastCommandSize(), APID_CMD_SUCC);

	//fill the data bytes
	//print the command information after the CCSDS header
	i_sprintf_ret = snprintf((char *)cmdSuccess + 11, 100, "%s\n", GetLastCommand());
	//check to make sure that the sizes match and we are reporting what we think we are
	if(i_sprintf_ret == GetLastCommandSize())
		status = CMD_SUCCESS;
	else
		status = CMD_FAILURE;
	//calculate the checksums
	CalculateChecksums(cmdSuccess, i_sprintf_ret);
	//send out the packet
	bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)cmdSuccess, (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE));
	if(bytes_sent == (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE))
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
 */
int reportFailure(XUartPs Uart_PS)
{
	int status = 0;
	int bytes_sent = 0;
	int i_sprintf_ret = 0;
	unsigned char *cmdFailure = malloc(100);
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
//				Note: the length should not account for the CCSDS Header;
//						Just give the length of the data bytes.
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

int CreatDefaultConfig(void)
{
	ConfigBuff = (CONFIG_STRUCT_TYPE){.TriggerThreshold=2,
	  	.ECutLoMod1=3.0f,
		.ECutHiMod1=3.0f,
		.ECutLoMod2=4.0f,
		.ECutHiMod2=4.0f,
		.ECutLoMod3=3.0f,
		.ECutHiMod3=3.0f,
		.ECutLoMod4=4.0f,
		.ECutHiMod4=4.0f,
		.PSDCutLoMod1=5.0f,
		.PSDCutHiMod1=6.0f,
		.PSDCutLoMod2=5.0f,
		.PSDCutHiMod2=6.0f,
		.PSDCutLoMod3=5.0f,
		.PSDCutHiMod3=6.0f,
		.PSDCutLoMod4=5.0f,
		.PSDCutHiMod4=6.0f,
		.WideECutLoMod1=3.0f,
		.WideECutHiMod1=3.0f,
		.WideECutLoMod2=4.0f,
		.WideECutHiMod2=4.0f,
		.WideECutLoMod3=3.0f,
		.WideECutHiMod3=3.0f,
		.WideECutLoMod4=4.0f,
		.WideECutHiMod4=4.0f,
		.WidePSDCutLoMod1=5.0f,
		.WidePSDCutHiMod1=6.0f,
		.WidePSDCutLoMod2=5.0f,
		.WidePSDCutHiMod2=6.0f,
		.WidePSDCutLoMod3=5.0f,
		.WidePSDCutHiMod3=6.0f,
		.WidePSDCutLoMod4=5.0f,
		.WidePSDCutHiMod4=6.0f,
	    .HighVoltageValue[0]=11,
		.HighVoltageValue[1]=11,
		.HighVoltageValue[2]=11,
		.HighVoltageValue[3]=11,
		.IntegrationBaseline=0,
		.IntegrationShort=35,
		.IntegrationLong=131,
		.IntegrationFull=1531,
		.ECalSlope=12.0f,
		.EcalIntercept=13.0f
	};
	return 0;
}

int InitConfig(void)
{
	uint NumBytesWr;
	uint NumBytesRd;
	FRESULT F_RetVal;
	int RetVal = 0;

	// check that config file exists
	if( f_stat( cConfigFile, &CnfFno) )
	{
		//Open and write to a new config file
		CreatDefaultConfig();
		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_WRITE|FA_OPEN_ALWAYS);
		if(F_RetVal == FR_OK)
			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
		filptr_cConfigFile += NumBytesWr;
		if(F_RetVal == FR_OK)
			F_RetVal = f_close(&ConfigFile);
	}
	else // The config file exists, read it
	{
		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_READ|FA_WRITE);	//open with read/write access
		if(F_RetVal == FR_OK)
			F_RetVal = f_lseek(&ConfigFile, 0);							//go to beginning of file
		if(F_RetVal == FR_OK)
			F_RetVal = f_read(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesRd);	//Read the config file into ConfigBuff
		if(F_RetVal == FR_OK)
			F_RetVal = f_close(&ConfigFile);							//close the file
	}

	RetVal = (int)F_RetVal;
	return RetVal;
}


int SaveConfig()
{
	uint NumBytesWr;
	FRESULT F_RetVal;
	int RetVal = 0;

	// check that config file exists
	if( f_stat( cConfigFile, &CnfFno) )
	{	// f_stat returns non-zero(false) if no file exists, so open/create the file
		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_WRITE|FA_OPEN_ALWAYS);
		if(F_RetVal == FR_OK)
			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
		filptr_cConfigFile += NumBytesWr;
		if(F_RetVal == FR_OK)
			F_RetVal = f_close(&ConfigFile);
	}
	else // If the file exists, write it
	{
		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_READ|FA_WRITE);	//open with read/write access
		if(F_RetVal == FR_OK)
			F_RetVal = f_lseek(&ConfigFile, 0);							//go to beginning of file
		if(F_RetVal == FR_OK)
			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);	//Write the ConfigBuff to config file
		if(F_RetVal == FR_OK)
			F_RetVal = f_close(&ConfigFile);							//close the file
		}

	RetVal = (int)F_RetVal;
    return RetVal;
}

