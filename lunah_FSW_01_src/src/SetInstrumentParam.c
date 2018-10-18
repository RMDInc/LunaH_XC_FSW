/*
 * SetInstrumentParam.c
 *
 *  Self terminating functions to set instrument parameters
 *
 *  Created on: Jun 20, 2018
 *      Author: IRDLAB
 */

#include <stdio.h>
#include <xparameters.h>
#include "lunah_defines.h"
#include "lunah_utils.h"
#include "xuartps.h"
#include "ff.h"
#include "LI2C_Interface.h"
#include "xiicps.h"

extern CONFIG_STRUCT_TYPE ConfigBuff;
extern XUartPs Uart_PS;
extern char cWriteToLogFile[];
extern int iSprintfReturn;
extern unsigned char error_buff[];
extern int err_buff_size;
extern int IIC_SLAVE_ADDR4;

/*
 * SetTriggerThreshold
 *    Set Event Trigger Threshold
 *        Syntax: SetTriggerThreshold(Threshold)
 * 		  �	Threshold = (Integer) A value between 0 � 10000
 *        Description: Change the instrument�s trigger threshold. This value is recorded as the new default value for the system.
 *        Latency: TBD
 *        Return: Threshold - Echoes back the value from the system.
 *	              �FFFFFF� - Indicates failure to set the threshold.
 *
 */
int SetTriggerThreshold(int iTrigThreshold)
{
  int bytes_sent;
  char report_buff[100];

	if((iTrigThreshold > 0) && (iTrigThreshold < 16000))				//check that it's within accepted values
	{
		Xil_Out32(XPAR_AXI_GPIO_10_BASEADDR, (u32)(iTrigThreshold));
		iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Set trigger threshold to %d ", iTrigThreshold);
		//write to log file

		//write to config file buffer
		ConfigBuff.TriggerThreshold = iTrigThreshold;
		// save config file
		SaveConfig();
		//read back value from the FPGA and echo to user
		iTrigThreshold = 0;	//reset var before reading
		iTrigThreshold = Xil_In32(XPAR_AXI_GPIO_10_BASEADDR);
		iSprintfReturn = snprintf(report_buff, 100, "%d\n", iTrigThreshold);
		bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);

	}
	else
	{
		bytes_sent = XUartPs_Send(&Uart_PS, error_buff, err_buff_size);
	}

    return(bytes_sent);
}

/*
 * Set Neutron Cut Gates
 *		Syntax: SetNeutronCutGates(ECut1, ECut2, PCut1, PCut2)
 *			�	ECut = (Float) floating point values between 0 � 200,000 MeV
 *			�	PCut = (Float)  point values between 0 � 3.0
 *		Description: Set the cuts on neutron energy (ECut) and psd spectrum (PCut) when calculating neutrons totals for the MNS_EVTS, MNS_CPS, and MNS_SOH data files. These values are recorded as the new default values for the system.
 * 		Latency: TBD
 *		Return: ECut1_ECut2_PCut1_PCut2 Echoes back the values currently set for the system.
 */
int SetNeutronCutGates(float ECut1, float ECut2, float PCut1, float PCut2)
{
  int bytes_sent;
  char report_buff[100];

      // write to config file buffer
      ConfigBuff.EnergyCut[0] = ECut1;
      ConfigBuff.EnergyCut[1] = ECut2;
      ConfigBuff.PsdCut[0] = PCut1;
      ConfigBuff.PsdCut[1] = PCut2;
      // Save Config file
      SaveConfig();
  	  //send return value for function
	  iSprintfReturn = snprintf(report_buff, 100, "%f_%f_%f_%f\n", ECut1, ECut2, PCut1, PCut2);
	  bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);

	  return(bytes_sent);
}

/*
 * Set Wide Neutron Cut Gates
 *		Syntax: SetWideNeuronCutGates(WideECut1, WideECut2, WidePCut1, WidePCut2
 *			�	ECut = (Float) floating point values between 0 � 200,000 MeV
 *			�	PCut = (Float)  point values between 0 � 3.0
 *		Description: Set the cuts on neutron energy (ECut) and psd spectrum (PCut) when calculating neutrons totals for the MNS_EVTS, MNS_CPS, and MNS_SOH data files. These values are recorded as the new default values for the system.
 * 		Latency: TBD
 *		Return: ECut1_ECut2_PCut1_PCut2 Echoes back the values currently set for the system.
 */
int SetWideNeutronCutGates(float WideECut1, float WideECut2, float WidePCut1, float WidePCut2)
{
	int bytes_sent;
	char report_buff[100];

	// write to config file buffer
	    ConfigBuff.EnergyCut[0] = WideECut1;
	    ConfigBuff.EnergyCut[1] = WideECut2;
	    ConfigBuff.PsdCut[0] = WidePCut1;
	    ConfigBuff.PsdCut[1] = WidePCut2;
	   // Save Config file
	    SaveConfig();
		//send return value for function
		iSprintfReturn = snprintf(report_buff, 100, "%f_%f_%f_%f\n", WideECut1, WideECut2, WidePCut1, WidePCut2);
		bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);

		return(bytes_sent);
}

/*
 * Set High Voltage  (note: connections to pot 2 and pot 3 are reversed - handled in the function)
 * ***********************this swap may need to be reversed, as the electronics (boards) may have been replaced!!!***********************
 * Syntax: SetHighVoltage(PMTID, Value)
 * �	PMTID = (Integer) PMT ID, 1 � 4, 5 to choose all tubes
 * �	Value =  (Integer) high voltage to set, 0 � 256 (not linearly mapped to volts)
 * Description: Set the bias voltage on any PMT in the array. The PMTs may be set individually or as a group.
 *
 */
int SetHighVoltage(int PmtId, int Value)
{
  unsigned char i2c_Send_Buffer[2];
  unsigned char i2c_Recv_Buffer[2];
  int cntrl = 10;  // write command
  int RetVal;
  int bytes_sent;
  char report_buff[100];

    // Fix swap of pot 2 and 3 connections if PmtId == 2 make it 3 and if PmtId ==3 make it 2
    if(PmtId & 0x2)
    {
        PmtId ^= 1;
    }

    i2c_Send_Buffer[0] = cntrl | (PmtId - 1);
    i2c_Send_Buffer[1] = Value;
    RetVal = IicPsMasterSend(IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, &IIC_SLAVE_ADDR4);  // check reason for pointer param 4
    if(RetVal != XST_SUCCESS)
    {
    	bytes_sent = XUartPs_Send(&Uart_PS, error_buff, err_buff_size);
    }
    else
    {

      // write to config file
    	ConfigBuff.HighVoltageValue[PmtId-1] = Value;
    	SaveConfig();
        iSprintfReturn = snprintf(report_buff, 100, "%d_%d\n", PmtId, Value);
        bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);
    }
    return(bytes_sent);
}

/*
 * SetIntergrationTime
 *    Set Integration Times
 *       Syntax: SetIntergrationTime(baseline, short, long ,full)
 *         	�	Values = (Signed Integer) values in microseconds
 *         	Description: Set the integration times for event-by-event data.
 *		   	Latency: TBD
 * 			Return: Baseline_short_long_full - Echoes back the values currently set for the system.
 *					�FFFFFF� - Indicates failure.
 */
int SetIntergrationTime(int Baseline, int Short, int Long, int Full)
{
  int bytes_sent;
  char report_buff[100];

    if((Baseline < Short) && ( Short < Long) && (Long < Full))	//if each is greater than the last
  	{
  		Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, ((u32)(Baseline+52)/4));
  		Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, ((u32)(Short+52)/4));
  		Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, ((u32)(Long+52)/4));
  		Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, ((u32)(Full+52)/4));

  		//write this to the log file
  		iSprintfReturn = snprintf(cWriteToLogFile, LOG_FILE_BUFF_SIZE, "Set integration times to %d %d %d %d ", Baseline, Short, Long, Full);
  		//write to log file function


  		// write to config
  		ConfigBuff.IntegrationBaseline = Baseline;
  		ConfigBuff.IntegrationShort = Short;
  		ConfigBuff.IntegrationLong = Long;
  		ConfigBuff.IntegrationFull = Full;
  		// save config
  		SaveConfig();
  		//send return value for function
  		iSprintfReturn = snprintf(report_buff, 100, "%d_%d_%d_%d\n",Baseline, Short, Long, Full);
  		bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);
  	}
  	else
  		bytes_sent = XUartPs_Send(&Uart_PS, error_buff, err_buff_size);

    return(bytes_sent);


}

/*
 * SetEnergyCalParms
 *      Set Energy Calibration Parameters
 *			Syntax: SetEnergyCalParam(Slope, Intercept)
 *				Slope = (Float) value for the slope
 * 				Intercept = (Float) value for the intercept
 * 			Description: These values modify the energy calculation based on the formula y = m*x + b, where m is the slope from above and b is the intercept. The default values are m = 1.0 and b = 0.0.
 * 			Latency: TBD
 *			Return: Slope_Intercept - Echoes back the values currently set for the system.
 *					�FFFFFF� - Indicates failure.
 */
int SetEnergyCalParam(float Slope, float Intercept)
{
  int bytes_sent;
  char report_buff[100];

    ConfigBuff.ECalSlope = Slope;
    ConfigBuff.EcalIntercept = Intercept;
    SaveConfig();
    //send return value for function
  	iSprintfReturn = snprintf(report_buff, 100, "%f_%f\n", Slope, Intercept);
  	bytes_sent = XUartPs_Send(&Uart_PS, (u8 *)report_buff, iSprintfReturn);

	return(bytes_sent);
}




