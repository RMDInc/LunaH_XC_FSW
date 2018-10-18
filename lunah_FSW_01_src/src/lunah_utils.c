/*
 * lunah_utils.c
 *
 *  Created on: Jun 22, 2018
 *      Author: IRDLAB
 */

//#include <xtime_l.h>
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

static float anlg_board_temp = 23;
static float digi_board_temp = 24;
static float modu_board_temp = 25;
static int iNeuronTotal = 50;
static int check_temp_sensor = 0;

extern XUartPs Uart_PS;
extern int IIC_SLAVE_ADDR1; //HV on the analog board - write to HV pots, RDAC
extern int IIC_SLAVE_ADDR2;	//Temp sensor on digital board
extern int IIC_SLAVE_ADDR3;	//Temp sensor on the analog board
extern int IIC_SLAVE_ADDR4;	//VTSET on the analog board - give voltage to TEC regulator
extern int IIC_SLAVE_ADDR5; //Extra Temp Sensor Board, on module near thermistor on TEC


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
 *  stub file to return neuron total.
 */
int GetNeuronTotal(void)
{
	return(iNeuronTotal);
}

int PutNeuronTotal(int total)
{
	iNeuronTotal = total;
	return iNeuronTotal;
}

int IncNeuronTotal(int increment)
{
    iNeuronTotal += increment;
	return iNeuronTotal;
}

/*
 *  CheckForSOH
 *      Check if time to send SOH and if it is send it.
 */
int CheckForSOH(void)
{
  int iNeuronTotal;

	XTime_GetTime(&LocalTimeCurrent);
	if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (LocalTime +  1))
	{
		iNeuronTotal = GetNeuronTotal();
		LocalTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND;
		report_SOH(LocalTime, iNeuronTotal, Uart_PS);
	}
	return LocalTime;
}



//////////////////////////// Report SOH Function ////////////////////////////////
//This function takes in the number of neutrons currently counted and the local time
// and pushes the SOH data product to the bus over the UART
int report_SOH(XTime local_time, int i_neutron_total, XUartPs Uart_PS)
{
	//Variables
	//change this to unsigned char and run on board
	unsigned char report_buff[100] = "";
	unsigned char i2c_Send_Buffer[2] = {};
	unsigned char i2c_Recv_Buffer[2] = {};
	int a = 0;
	int b = 0;
	//int analog_board_temp = 0;
	//int digital_board_temp = 0;
	int i_sprintf_ret = 0;
	int *IIC_SLAVE_ADDR;		//pointer to slave

/*	//analog board temp - case 14
	IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR3;
	i2c_Send_Buffer[0] = 0x0;
	i2c_Send_Buffer[1] = 0x0;
	IicPsMasterSend(IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
	IicPsMasterRecieve(IIC_DEVICE_ID_0, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
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
	analog_board_temp = b; */

	//if temp has not been checked in 2s, add 0.5 degrees to the temp
	//if check_temp_sensor is 0, check analog board temp sensor, else check the next sensor
	// then increment check_temp_sensor and reset time to next check
	switch(check_temp_sensor){
	case 0:
		XTime_GetTime(&LocalTimeCurrent);
		if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (TempTime + 2))
		{
			TempTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND; //temp time is reset
			check_temp_sensor++;
			anlg_board_temp += 0.5;
		}
		break;
	case 1:
		XTime_GetTime(&LocalTimeCurrent);
		if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (TempTime + 2))
		{
			TempTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND; //temp time is reset
			check_temp_sensor++;
			digi_board_temp += 0.5;
		}
		break;
	case 2:
		XTime_GetTime(&LocalTimeCurrent);
		if(((LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND) >= (TempTime + 2))
		{
			TempTime = (LocalTimeCurrent - LocalTimeStart)/COUNTS_PER_SECOND; //temp time is reset
			check_temp_sensor = 0;
			modu_board_temp += 0.5;
		}
		break;
	default:
		xil_printf("a problem\r\n");
		break;
	}


/*	//digital board temp - case 13
	IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR2;
	IicPsMasterSend(IIC_DEVICE_ID_1, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
	IicPsMasterRecieve(IIC_DEVICE_ID_1, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
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
	digital_board_temp = b; */


	//print the SOH information after the CCSDS header
	i_sprintf_ret = snprintf((char *)report_buff + 11, 100, "%2.2f\t%2.2f\t%2.2f\t%d\t%llu\n", anlg_board_temp, digi_board_temp, modu_board_temp, i_neutron_total, local_time);
	//Put in the CCSDS Header
	PutCCSDSHeader(report_buff, i_sprintf_ret);
	//calculate the checksums
	CalculateChecksums(report_buff, i_sprintf_ret);
	XUartPs_Send(&Uart_PS, (u8 *)report_buff, (i_sprintf_ret + CCSDS_HEADER_SIZE + CHECKSUM_SIZE));

	return check_temp_sensor;
}

void PutCCSDSHeader(unsigned char * SOH_buff, int length)
{
	//get the values for the CCSDS header
	SOH_buff[0] = 0x35;
	SOH_buff[1] = 0x2E;
	SOH_buff[2] = 0xF8;
	SOH_buff[3] = 0x53;
	SOH_buff[4] = 0x0A; //identify detector 0 or 1
	SOH_buff[5] = 0x22;	//APID for SOH
	SOH_buff[6] = 0xC0;
	SOH_buff[7] = 0x01;
	//add in the checksums to the length
	//To calculate the length of the packet, we need to add all the bytes in the MiniNS-data
	// plus the checksums (4 bytes) plus the reset request byte (1 byte)
	// then we subtract one byte
	length += 4;
	SOH_buff[8] = (length & 0xFF00) >> 8;
	SOH_buff[9] = length & 0xFF;
	SOH_buff[10] = 0x00;

	return;
}
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
	  	.EnergyCut[0]=3.0f,.EnergyCut[1]=4.0f,
		.PsdCut[0]=5.0f,.PsdCut[1]=6.0f,
		.WideEnergyCut[0]=7.0f,.WideEnergyCut[1]=8.0f,
		.WidePsdCut[0]=9.0f,.WidePsdCut[1]=10.0f,
	    .HighVoltageValue[0]=11,.HighVoltageValue[1]=11,.HighVoltageValue[2]=11,.HighVoltageValue[3]=11,
		.IntegrationBaseline=0,.IntegrationShort=35,.IntegrationLong=131,.IntegrationFull=1531,
		.ECalSlope=12.0f,.EcalIntercept=13.0f
	};
	return 0;
}

int AltCreatDefaultConfig(void)
{
	CONFIG_STRUCT_TYPE DefaultConfigValues = {.TriggerThreshold=2,
		  	.EnergyCut[0]=3.0f,.EnergyCut[1]=4.0f,
			.PsdCut[0]=5.0f,.PsdCut[1]=6.0f,
			.WideEnergyCut[0]=7.0f,.WideEnergyCut[1]=8.0f,
			.WidePsdCut[0]=9.0f,.WidePsdCut[1]=10.0f,
		    .HighVoltageValue[0]=11,.HighVoltageValue[1]=11,.HighVoltageValue[2]=11,.HighVoltageValue[3]=11,
			.IntegrationBaseline=0,.IntegrationShort=35,.IntegrationLong=131,.IntegrationFull=1531,
			.ECalSlope=12.0f,.EcalIntercept=13.0f};

	ConfigBuff = DefaultConfigValues;
	return 0;
}

// #define INIT_CONFIG_TESTED
int InitConfig(void)
{
  uint NumBytesWr;
  uint NumBytesRd;
  FRESULT F_RetVal;
//  int RetVal;
#define INIT_CONFIG_TESTED
#ifndef INIT_CONFIG_TESTED
     return 0;
#else
	// check that config file exists
	if( f_stat( cConfigFile, &CnfFno) )
	{	// f_stat returns non-zero(false) if no file exists, create default buffer and open/create the file
		CreatDefaultConfig();
		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_WRITE|FA_OPEN_ALWAYS);
		F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
		filptr_cConfigFile += NumBytesWr;
		F_RetVal = f_close(&ConfigFile);
	}
	else // If the file exists, read it
	{
		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_READ|FA_WRITE);	//open with read/write access
		F_RetVal = f_lseek(&ConfigFile, 0);							//go to beginning of file
		F_RetVal = f_read(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesRd);	//Read the config file into ConfigBuff

		F_RetVal = f_close(&ConfigFile);							//close the file
	}
#endif
	return 0;
}

#define SAVE_CONFIG_TESTED
int SaveConfig()
{
	uint NumBytesWr;
	FRESULT F_RetVal;
	int RetVal = 0;

#ifndef SAVE_CONFIG_TESTED
	    return 0;
#else

		// check that config file exists
		if( f_stat( cConfigFile, &CnfFno) )
		{	// f_stat returns non-zero(false) if no file exists, so open/create the file
			F_RetVal = f_open(&ConfigFile, cConfigFile, FA_WRITE|FA_OPEN_ALWAYS);
			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
			filptr_cConfigFile += NumBytesWr;
			F_RetVal = f_close(&ConfigFile);
		}
		else // If the file exists, write it
		{
			F_RetVal = f_open(&ConfigFile, cConfigFile, FA_READ|FA_WRITE);	//open with read/write access
			F_RetVal = f_lseek(&ConfigFile, 0);							//go to beginning of file
			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);	//Write the ConfigBuff to config file

			F_RetVal = f_close(&ConfigFile);							//close the file
		}




    return RetVal;
#endif
}

