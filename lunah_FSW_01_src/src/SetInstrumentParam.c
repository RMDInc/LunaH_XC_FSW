/*
 * SetInstrumentParam.c
 *
 *  Self terminating functions to set instrument parameters
 *
 *  Created on: Jun 20, 2018
 *      Author: IRDLAB
 */

#include "SetInstrumentParam.h"

extern CONFIG_STRUCT_TYPE ConfigBuff;

/*
 *    Set Event Trigger Threshold
 * 		  Threshold = (Integer) A value between 0 - 16000
 *        Description: 	Change the instruments trigger threshold.
 *        				This value is recorded as the new default value for the system.
 *        Latency: TBD
 *        Return: command SUCCESS (0) or command FAILURE (1)
 *
 */
int SetTriggerThreshold(int iTrigThreshold)
{
	int status = 0;	//0=SUCCESS, 1=FAILURE

	//check that it's within accepted values
	if((iTrigThreshold > 0) && (iTrigThreshold < 16000))
	{
		//set the threshold in the FPGA
		Xil_Out32(XPAR_AXI_GPIO_10_BASEADDR, (u32)(iTrigThreshold));
		//read back value from the FPGA and compare with intended change
		if(iTrigThreshold == Xil_In32(XPAR_AXI_GPIO_10_BASEADDR))
		{
			//write to config file buffer
			ConfigBuff.TriggerThreshold = iTrigThreshold;
			// save config file
			SaveConfig();

			status = CMD_SUCCESS;
		}
		else
			status = CMD_FAILURE;	//indicate we did not set the threshold appropriately
	}
	else //values were not within acceptable range
	{
		status = CMD_FAILURE;
	}

	return status;
}

/*
 * SetEnergyCalParams
 *      Set Energy Calibration Parameters
 *		Syntax: SetEnergyCalParam(Slope, Intercept)
 *			Slope = (Float) value for the slope
 * 			Intercept = (Float) value for the intercept
 * 		Description:  These values modify the energy calculation based on the formula y = m*x + b,
 * 					  where m is the slope from above and b is the intercept.
 * 					  The default values are m (Slope) = 1.0 and b (Intercept) = 0.0
 * 		Latency: TBD
 *      Return: command SUCCESS (0) or command FAILURE (1)
 */
int SetEnergyCalParam(float Slope, float Intercept)
{
	int status = 0;

	//check that it's within accepted values
	if((Slope >= 0.0) && (Slope <= 10.0))
	{
		//check that it's within accepted values
		if((Intercept > -100.0) && (Intercept <= 100.0))
		{
			ConfigBuff.ECalSlope = Slope;
			ConfigBuff.EcalIntercept = Intercept;
			SaveConfig();

			status = CMD_SUCCESS;
		}
		else
			status = CMD_FAILURE;
	}
	else
		status = CMD_FAILURE;
	return status;
}

/*
 * Set Neutron Cut Gates
 *		Syntax: SetNeutronCutGates(ModuleNumber, ECut1, ECut2, PCut1, PCut2)
 *			Module Number = (Int) assigns the cut values to a specific CLYC module
 *			ECut = (Float) floating point values between 0 - 200,000 keV
 *			PCut = (Float) floating point values between 0 - 2.0
 *		Description:  Set the cuts on neutron energy (ECut) and psd spectrum (PCut)
 *					  when calculating neutron totals for the MNS_EVTS, MNS_CPS,
 *					  and MNS_SOH data files.
 * 		Latency: TBD
 *		Return: command SUCCESS (0) or command FAILURE (1)
 *
 *		NB: We may not use these functions anymore because of a change in how we are going to
 *		implement these cuts. The new approach is to re-calculate the PSD/Energy cuts for each
 *		module every time we check the temperature during DAQ. Thus, we don't need to set these
 *		by hand.
 *
 *		What this will be useful for entering is the conversion table which will be referenced
 *		during DAQ.
 */
int SetNeutronCutGates(int moduleID, float ECut1, float ECut2, float PCut1, float PCut2)
{
	int status = 0;
	if((ECut1 < ECut2) && (PCut1 < PCut2))
	{
		if( (ECut1 >= 0.0) && (ECut2 <= 200000))
		{
			if( (PCut1 >= 0.0) && (PCut2 <= 2.0))
			{
				//Values are within acceptable ranges
				// write to config file buffer
				switch(moduleID)
				{
				case 0:
					//module 0
					ConfigBuff.ECutLoMod1 = ECut1;
					ConfigBuff.ECutHiMod1 = ECut2;
					ConfigBuff.PSDCutLoMod1 = PCut1;
					ConfigBuff.PSDCutHiMod1 = PCut2;
					break;
				case 1:
					//module 1
					ConfigBuff.ECutLoMod2 = ECut1;
					ConfigBuff.ECutHiMod2 = ECut2;
					ConfigBuff.PSDCutLoMod2 = PCut1;
					ConfigBuff.PSDCutHiMod2 = PCut2;
					break;
				case 2:
					//module 2
					ConfigBuff.ECutLoMod3 = ECut1;
					ConfigBuff.ECutHiMod3 = ECut2;
					ConfigBuff.PSDCutLoMod3 = PCut1;
					ConfigBuff.PSDCutHiMod3 = PCut2;
					break;
				case 3:
					//module 3
					ConfigBuff.ECutLoMod4 = ECut1;
					ConfigBuff.ECutHiMod4 = ECut2;
					ConfigBuff.PSDCutLoMod4 = PCut1;
					ConfigBuff.PSDCutHiMod4 = PCut2;
					break;
				default:
					//bad value for the module ID, just use the defaults
//					ConfigBuff.ECutLoMod1 = ECut1;
//					ConfigBuff.ECutHiMod1 = ECut2;
//					ConfigBuff.PSDCutLoMod1 = PCut1;
//					ConfigBuff.PSDCutHiMod1 = PCut2;
					break;
				}
				// Save Config file
				SaveConfig();
				status = CMD_SUCCESS;
			}
			else
				status = CMD_FAILURE;
		}
		else
			status = CMD_FAILURE;
	}
	else
		status = CMD_FAILURE;

	return status;
}

/*
 * Set Wide Neutron Cut Gates
 *		Syntax: SetWideNeuronCutGates(ModuleNumber, WideECut1, WideECut2, WidePCut1, WidePCut2)
 *			Module Number = (Int) assigns the cut values to a specific CLYC module
 *			ECut = (Float) floating point values between 0 � 200,000 MeV
 *			PCut = (Float)  point values between 0 � 3.0
 *		Description: Set the cuts on neutron energy (ECut) and psd spectrum (PCut) when calculating neutrons totals for the MNS_EVTS, MNS_CPS, and MNS_SOH data files. These values are recorded as the new default values for the system.
 * 		Latency: TBD
 *		Return: command SUCCESS (0) or command FAILURE (1)
 */
int SetWideNeutronCutGates(int moduleID, float WideECut1, float WideECut2, float WidePCut1, float WidePCut2)
{
	int status = 0;
	if((WideECut1 < WideECut2) && (WidePCut1 < WidePCut2))
		{
			if( (WideECut1 >= 0.0) && (WideECut2 <= 200000))
			{
				if( (WidePCut1 >= 0.0) && (WidePCut2 <= 2.0))
				{
					// write to config file buffer
					switch(moduleID)
					{
					case 0:
						//module 0
						ConfigBuff.WideECutLoMod1 = WideECut1;
						ConfigBuff.WideECutHiMod1 = WideECut2;
						ConfigBuff.WidePSDCutLoMod1 = WidePCut1;
						ConfigBuff.WidePSDCutHiMod1 = WidePCut2;
						break;
					case 1:
						//module 1
						ConfigBuff.WideECutLoMod2 = WideECut1;
						ConfigBuff.WideECutHiMod2 = WideECut2;
						ConfigBuff.WidePSDCutLoMod2 = WidePCut1;
						ConfigBuff.WidePSDCutHiMod2 = WidePCut2;
						break;
					case 2:
						//module 2
						ConfigBuff.WideECutLoMod3 = WideECut1;
						ConfigBuff.WideECutHiMod3 = WideECut2;
						ConfigBuff.WidePSDCutLoMod3 = WidePCut1;
						ConfigBuff.WidePSDCutHiMod3 = WidePCut2;
						break;
					case 3:
						//module 3
						ConfigBuff.WideECutLoMod4 = WideECut1;
						ConfigBuff.WideECutHiMod4 = WideECut2;
						ConfigBuff.WidePSDCutLoMod4 = WidePCut1;
						ConfigBuff.WidePSDCutHiMod4 = WidePCut2;
						break;
					default:
						//bad value for the module ID, just use the defaults
	//					ConfigBuff.ECutLoMod1 = ECut1;
	//					ConfigBuff.ECutHiMod1 = ECut2;
	//					ConfigBuff.PSDCutLoMod1 = PCut1;
	//					ConfigBuff.PSDCutHiMod1 = PCut2;
						break;
					}
					// Save Config file
					SaveConfig();
					status = CMD_SUCCESS;
				}
				else
					status = CMD_FAILURE;
			}
			else
				status = CMD_FAILURE;
		}
		else
			status = CMD_FAILURE;

	return status;
}

/*
 * Set High Voltage  (note: connections to pot 2 and pot 3 are reversed - handled in the function)
 * ***********************this swap may need to be reversed, as the electronics (boards) may have been replaced!!!***********************
 * 		Syntax: SetHighVoltage(PMTID, Value)
 * 			PMTID = (Integer) PMT ID, 1 - 4, 5 to choose all tubes
 * 			Value = (Integer) high voltage to set, 0 - 256 (not linearly mapped to volts)
 * 		Description: Set the bias voltage on any PMT in the array. The PMTs may be set individually or as a group.
 *			Latency: TBD
 *			Return: command SUCCESS (0) or command FAILURE (1)
 */
int SetHighVoltage(XIicPs Iic, unsigned char PmtId, int Value)
{
	int IIC_SLAVE_ADDR1 = 0x20; //HV on the analog board - write to HV pots, RDAC
	unsigned char i2c_Send_Buffer[2];
	unsigned char i2c_Recv_Buffer[2];
	unsigned char cntrl = 16;  // write command
	int RetVal = 0;
	int status = 0;
	int iterator = 0;

	// Fix swap of pot 2 and 3 connections if PmtId == 2 make it 3 and if PmtId ==3 make it 2
	if(PmtId & 0x2)
	{
		PmtId ^= 1;
	}

	//check the PMT ID is ok
	if((PmtId > 0) && (PmtId <= 5))
	{
		//check tap value is an acceptable number
		if((Value >= 0) && (Value <= 255))
		{
			//We just want to do a single tube
			if(PmtId != 5)
			{
				//create the send buffer
				i2c_Send_Buffer[0] = cntrl | (PmtId - 1);
				i2c_Send_Buffer[1] = Value;
				//send the command to the HV
				RetVal = IicPsMasterSend(Iic, IIC_DEVICE_ID_0, i2c_Send_Buffer, i2c_Recv_Buffer, &IIC_SLAVE_ADDR1);
				if(RetVal == XST_SUCCESS)
				{
					// write to config file
					ConfigBuff.HighVoltageValue[PmtId-1] = Value;
					SaveConfig();
					status = CMD_SUCCESS;
				}
				else
					status = CMD_FAILURE;
			}
			else if(PmtId == 5)
			{
				//do the above code for each PMT
				for(iterator = 1; iterator < 5; iterator++)
				{
					//cycle over PmtId 0, 1, 2, 3 to set the voltage on each PMT
					PmtId = iterator;
					//create the send buffer
					i2c_Send_Buffer[0] = cntrl | (PmtId - 1);
					i2c_Send_Buffer[1] = Value;
					//send the command to the HV
					RetVal = IicPsMasterSend(Iic, IIC_DEVICE_ID_0 ,i2c_Send_Buffer, i2c_Recv_Buffer, &IIC_SLAVE_ADDR1);
					if(RetVal == XST_SUCCESS)
					{
						// write to config file
						ConfigBuff.HighVoltageValue[PmtId-1] = Value;
						SaveConfig();
						status = CMD_SUCCESS;
					}
					else
					{
						status = CMD_FAILURE;
						break;
					}
				}
			}
			else
				status = CMD_FAILURE;
		}
		else
			status = CMD_FAILURE;
	}
	else
		status = CMD_FAILURE;

	return status;
}

/*
 * SetIntergrationTime
 * 		Set Integration Times
 * 		Syntax: SetIntergrationTime(baseline, short, long ,full)
 * 			Values = (Signed Integer) values in microseconds
 * 		Description: Set the integration times for event-by-event data.
 *		Latency: TBD
 *		Return: command SUCCESS (0) or command FAILURE (1)
 */
int SetIntergrationTime(int Baseline, int Short, int Long, int Full)
{
	int status = 0;

	if((Baseline < Short) && ( Short < Long) && (Long < Full))	//if each is greater than the last
	{
		//set the values provided
		Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, ((u32)(Baseline+52)/4));
		Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, ((u32)(Short+52)/4));
		Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, ((u32)(Long+52)/4));
		Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, ((u32)(Full+52)/4));

		//error check to make sure that we have set all the values correctly
		//we can worry about error checking later on in the FSW process
		//read back the values from the FPGA //They should be equal to the values we set
		if(Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) == Baseline)
			if(Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) == Short)
				if(Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) == Long)
					if(Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) == Full)
					{
						// Write to config
						ConfigBuff.IntegrationBaseline = Baseline;
						ConfigBuff.IntegrationShort = Short;
						ConfigBuff.IntegrationLong = Long;
						ConfigBuff.IntegrationFull = Full;
						// Save config
						SaveConfig();

						status = CMD_SUCCESS;
					}
					else
						status = CMD_FAILURE;
				else
					status = CMD_FAILURE;
			else
				status = CMD_FAILURE;
		else
			status = CMD_FAILURE;

		//if this comes back as CMD_FAILURE, try and set the default value
		//later

		status = CMD_SUCCESS;
	}
	else
		status = CMD_FAILURE;

	return status;


}
