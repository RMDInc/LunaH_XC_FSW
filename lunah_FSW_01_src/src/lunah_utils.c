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

CONFIG_STRUCT_TYPE ConfigBuff;

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
	b /= 16;
	analog_board_temp = b;

	//digital board temp - case 13
	IIC_SLAVE_ADDR=&IIC_SLAVE_ADDR2;
	IicPsMasterSend(IIC_DEVICE_ID_1, i2c_Send_Buffer, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
	IicPsMasterRecieve(IIC_DEVICE_ID_1, i2c_Recv_Buffer, IIC_SLAVE_ADDR);
	a = i2c_Recv_Buffer[0]<< 5;
	b = a | i2c_Recv_Buffer[1] >> 3;
	b /= 16;
	digital_board_temp = b;

	i_sprintf_ret = snprintf(report_buff, 100, "%d_%d_%u_%llu\n", analog_board_temp, digital_board_temp, i_neutron_total, local_time);
	XUartPs_Send(&Uart_PS, (u8 *)report_buff, i_sprintf_ret);

	return 0;
}

int ls_commnad(void)
{




    return 0;
}

int SaveConfigFile()
{





}

