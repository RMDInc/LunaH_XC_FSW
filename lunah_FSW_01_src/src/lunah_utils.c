/*
 * lunah_utils.c
 *
 *  Created on: Jun 22, 2018
 *      Author: IRDLAB
 */

#include <xtime_l.h>
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
static XTime LocalTimeStart;
static XTime LocalTimeCurrent = 0;

static int iNeuronTotal = 50;

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
	char report_buff[100] = "";
	unsigned char i2c_Send_Buffer[2] = {};
	unsigned char i2c_Recv_Buffer[2] = {};
	int a = 0;
	int b = 0;
	int analog_board_temp = 0;
	int digital_board_temp = 0;
	int i_sprintf_ret = 0;
	int *IIC_SLAVE_ADDR;		//pointer to slave

	//analog board temp - case 14
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
	analog_board_temp = b;

	//digital board temp - case 13
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
	digital_board_temp = b;

	i_sprintf_ret = snprintf(report_buff, 100, "%d_%d_%u_%llu\n", analog_board_temp, digital_board_temp, i_neutron_total, local_time);
	XUartPs_Send(&Uart_PS, (u8 *)report_buff, i_sprintf_ret);

	return 0;
}

int ls_commnad(void)
{




    return 0;
}

int CreatDefaultConfig(void)
{
	ConfigBuff = (CONFIG_STRUCT_TYPE){.ConfigLen=1,.TriggerThreshold=2,
	  	.EnergyCut[0]=3.0f,.EnergyCut[1]=4.0f,.PsdCut[0]=5.0f,.PsdCut[1]=6.0f,
		.WideEnergyCut[0]=7.0f,.WideEnergyCut[1]=8.0f,.WidePsdCut[0]=9.0f,.WidePsdCut[1]=10.0f,
	    .HighVoltageValue[0]=11,.HighVoltageValue[1]=11,.HighVoltageValue[2]=11,.HighVoltageValue[3]=11,
		.IntegrationBaseline=0,.IntegrationShort=35,.IntegrationLong=131,.IntegrationFull=1531,
		.ECalSlope=12.0f,.EcalIntercept=13.0f,.ConfigChecksum=14};
}

int AltCreatDefaultConfig(void)
{
  CONFIG_STRUCT_TYPE DefaultConfigValues = {.ConfigLen=1,.TriggerThreshold=2,
		  	.EnergyCut[0]=3.0f,.EnergyCut[1]=4.0f,.PsdCut[0]=5.0f,.PsdCut[1]=6.0f,
			.WideEnergyCut[0]=7.0f,.WideEnergyCut[1]=8.0f,.WidePsdCut[0]=9.0f,.WidePsdCut[1]=10.0f,
		    .HighVoltageValue[0]=11,.HighVoltageValue[1]=11,.HighVoltageValue[2]=11,.HighVoltageValue[3]=11,
			.IntegrationBaseline=0,.IntegrationShort=35,.IntegrationLong=131,.IntegrationFull=1531,
			.ECalSlope=12.0f,.EcalIntercept=13.0f,.ConfigChecksum=14};

	ConfigBuff = DefaultConfigValues;

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
}

#define SAVE_CONFIG_TESTED
int SaveConfig()
{
	uint NumBytesWr;
	FRESULT F_RetVal;
	int RetVal;

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

