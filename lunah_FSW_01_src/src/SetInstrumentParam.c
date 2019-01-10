/*
 * SetInstrumentParam.c
 *
 *  Self terminating functions to set instrument parameters
 *
 *  Created on: Jun 20, 2018
 *      Author: IRDLAB
 */

#include "SetInstrumentParam.h"

//File-Scope Variables
static char cConfigFile[] = "0:/ConfigFile.cnf";
static CONFIG_STRUCT_TYPE ConfigBuff;

/* This can be called for two different reasons:
 *  1.) When there is no config file on the SD card, this holds the default (hard coded)
 *  system parameters which are to be used until changed by the user.
 *  2.) When we want to reset the values in the config file
 *
 * This function will set all the values in the Config Buffer (file scope static buffer)
 *  to their original values.
 *
 * @param	None
 *
 * @return	None
 *
 */
void CreateDefaultConfig( void )
{
	//initialize the struct with all default values
	ConfigBuff = (CONFIG_STRUCT_TYPE){
		.ECalSlope=1.0,
		.EcalIntercept=0.0,
		.TriggerThreshold=9000,
		.IntegrationBaseline=0,
		.IntegrationShort=35,
		.IntegrationLong=131,
		.IntegrationFull=1531,
		.HighVoltageValue[0]=11,
		.HighVoltageValue[1]=11,
		.HighVoltageValue[2]=11,
		.HighVoltageValue[3]=11,
		.ScaleFactorEnergy_1_1=1.0,
		.ScaleFactorEnergy_1_2=1.0,
		.ScaleFactorEnergy_2_1=1.0,
		.ScaleFactorEnergy_2_2=1.0,
		.ScaleFactorEnergy_3_1=1.0,
		.ScaleFactorEnergy_3_2=1.0,
		.ScaleFactorEnergy_4_1=1.0,
		.ScaleFactorEnergy_4_2=1.0,
		.ScaleFactorPSD_1_1=1.0,
		.ScaleFactorPSD_1_2=1.0,
		.ScaleFactorPSD_2_1=1.0,
		.ScaleFactorPSD_2_2=1.0,
		.ScaleFactorPSD_3_1=1.0,
		.ScaleFactorPSD_3_2=1.0,
		.ScaleFactorPSD_4_1=1.0,
		.ScaleFactorPSD_4_2=1.0,
		.OffsetEnergy_1_1=0.0,
		.OffsetEnergy_1_2=0.0,
		.OffsetEnergy_2_1=0.0,
		.OffsetEnergy_2_2=0.0,
		.OffsetEnergy_3_1=0.0,
		.OffsetEnergy_3_2=0.0,
		.OffsetEnergy_4_1=0.0,
		.OffsetEnergy_4_2=0.0,
		.OffsetPSD_1_1=0.0,
		.OffsetPSD_1_2=0.0,
		.OffsetPSD_2_1=0.0,
		.OffsetPSD_2_2=0.0,
		.OffsetPSD_3_1=0.0,
		.OffsetPSD_3_2=0.0,
		.OffsetPSD_4_1=0.0,
		.OffsetPSD_4_2=0.0
	};

	return;
}

/* This function handles initializing the system with the values from the config file.
 * If no config file exists, one will be created using the default (hard coded) values
 *  available to the system.
 * If the config file is found on the SD card, then it is read in and values are assigned
 *
 * @param	None
 *
 * @return	FR_OK (0) or command FAILURE (!0)
 *
 */
int InitConfig(void)
{
	uint NumBytesWr;
	uint NumBytesRd;
	FRESULT F_RetVal;
	FILINFO CnfFno;
	FIL ConfigFile;
	int RetVal = 0;
	int ConfigSize = sizeof(ConfigBuff);

	// check if the config file exists
	if( f_stat( cConfigFile, &CnfFno) )
	{
		//Fill the Config Buffer with the default values
		CreateDefaultConfig();
		//Open and write to a new config file
		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_WRITE|FA_OPEN_ALWAYS);
		if(F_RetVal == FR_OK)
			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
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

/* This function will save the current system configuration to the configuration file, if it exists.
 * If no config file exists, then this function will create it and fill it with the current
 *  configuration structure.
 *
 *
 * @param	None
 *
 * @return	FR_OK (0) or command FAILURE (!0)
 *
 */
int SaveConfig()
{
	uint NumBytesWr;
	FRESULT F_RetVal;
	FILINFO CnfFno;
	FIL ConfigFile;
	int RetVal = 0;
	int ConfigSize = sizeof(ConfigBuff);

	// check that config file exists
	if( f_stat( cConfigFile, &CnfFno) )
	{	// f_stat returns non-zero(false) if no file exists, so open/create the file
		F_RetVal = f_open(&ConfigFile, cConfigFile, FA_WRITE|FA_OPEN_ALWAYS);
		if(F_RetVal == FR_OK)
			F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
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
 * Set the cuts on neutron energy(E) and psd ratio(P) when calculating neutron
 *  totals for the MNS_EVTS, MNS_CPS, and MNS_SOH data files.
 * These will be values to modify the elliptical cuts being placed on the events read in.
 * There are two ellipses by default, one at 1 sigma and one at 2 sigma, but the location in the E-P phase
 *  space can be modified by these parameters. This function allows the user to shift the centroid
 *  around (offsetE/P) in the E-P space or scale the size of the ellipses larger or smaller (scaleE/P).
 *
 * @param moduleID	Assigns the cut values to a specific CLYC module
 * 					Valid input range: 0 - 3
 * @param ellipseNum	Assigns the cut values to a specific ellipse for the module chosen
 * 						Valid input range: 1, 2
 * @param scaleE/scaleP	Scale the size of the ellipse being used to cut neutrons in the data
 * 						Valid input range: 0 - 25.5,
 * @param offsetE/offsetP	Move the centroid of the bounding ellipse around on the plot
 * 							Valid input range, E: 0 - 10 MeV (0-200,000 keV?)
 * 							Valid input range, P: 0 - 2
 * Latency: TBD
 *
 * Return: command SUCCESS (0) or command FAILURE (1)
 *
 */
int SetNeutronCutGates(int moduleID, int ellipseNum, float scaleE, float scaleP, float offsetE, float offsetP)
{
	int status = 0;

	//what do we need to check with the parameter input?
	//check that the scale factors are not 0
	if(scaleE != 0 && scaleP != 0)
	{
		if(offsetE)
		{
			//do these checks even make sense?
		}
	}
	//check that the offsets won't move the centroid to a different quadrant

	switch(moduleID)
	{
	case 0:	//module 0
		switch(ellipseNum)
		{
		case 1:
			ConfigBuff.ScaleFactorEnergy_1_1 = scaleE;
			ConfigBuff.ScaleFactorPSD_1_1 = scaleP;
			ConfigBuff.OffsetEnergy_1_1 = offsetE;
			ConfigBuff.OffsetPSD_1_1 = offsetP;
			break;
		case 2:
			ConfigBuff.ScaleFactorEnergy_1_2 = scaleE;
			ConfigBuff.ScaleFactorPSD_1_2 = scaleP;
			ConfigBuff.OffsetEnergy_1_2 = offsetE;
			ConfigBuff.OffsetPSD_1_2 = offsetP;
			break;
		default:
			status = CMD_FAILURE;
			break;
		}
		break;
	case 1:	//module 1
		switch(ellipseNum)
		{
		case 1:
			ConfigBuff.ScaleFactorEnergy_2_1 = scaleE;
			ConfigBuff.ScaleFactorPSD_2_1 = scaleP;
			ConfigBuff.OffsetEnergy_2_1 = offsetE;
			ConfigBuff.OffsetPSD_2_1 = offsetP;
			break;
		case 2:
			ConfigBuff.ScaleFactorEnergy_2_2 = scaleE;
			ConfigBuff.ScaleFactorPSD_2_2 = scaleP;
			ConfigBuff.OffsetEnergy_2_2 = offsetE;
			ConfigBuff.OffsetPSD_2_2 = offsetP;
			break;
		default:
			status = CMD_FAILURE;
			break;
		}
		break;
	case 2:	//module 2
		switch(ellipseNum)
		{
		case 1:
			ConfigBuff.ScaleFactorEnergy_3_1 = scaleE;
			ConfigBuff.ScaleFactorPSD_3_1 = scaleP;
			ConfigBuff.OffsetEnergy_3_1 = offsetE;
			ConfigBuff.OffsetPSD_3_1 = offsetP;
			break;
		case 2:
			ConfigBuff.ScaleFactorEnergy_3_2 = scaleE;
			ConfigBuff.ScaleFactorPSD_3_2 = scaleP;
			ConfigBuff.OffsetEnergy_3_2 = offsetE;
			ConfigBuff.OffsetPSD_3_2 = offsetP;
			break;
		default:
			status = CMD_FAILURE;
			break;
		}
		break;
	case 3:	//module 3
		switch(ellipseNum)
		{
		case 1:
			ConfigBuff.ScaleFactorEnergy_4_1 = scaleE;
			ConfigBuff.ScaleFactorPSD_4_1 = scaleP;
			ConfigBuff.OffsetEnergy_4_1 = offsetE;
			ConfigBuff.OffsetPSD_4_1 = offsetP;
			break;
		case 2:
			ConfigBuff.ScaleFactorEnergy_4_2 = scaleE;
			ConfigBuff.ScaleFactorPSD_4_2 = scaleP;
			ConfigBuff.OffsetEnergy_4_2 = offsetE;
			ConfigBuff.OffsetPSD_4_2 = offsetP;
			break;
		default:
			status = CMD_FAILURE;
			break;
		}
	default: //bad value for the module ID, just use the defaults
		//what are the defaults?
		break;
	}
	// Save Config file
	SaveConfig();
	status = CMD_SUCCESS;



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
/*int SetWideNeutronCutGates(int moduleID, int ellipseNum, float scaleE, float scaleP, float offsetE, float offsetP)
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
						ConfigBuff.ScaleFactorEnergy_1_2 = scaleE;
						ConfigBuff.ScaleFactorPSD_1_2 = scaleP;
						ConfigBuff.OffsetEnergy_1_2 = offsetE;
						ConfigBuff.OffsetPSD_1_2 = offsetP;
						break;
					case 1:
						//module 1
						ConfigBuff.ScaleFactorEnergy_2_2 = scaleE;
						ConfigBuff.ScaleFactorPSD_2_2 = scaleP;
						ConfigBuff.OffsetEnergy_2_2 = offsetE;
						ConfigBuff.OffsetPSD_2_2 = offsetP;
						break;
					case 2:
						//module 2
						ConfigBuff.ScaleFactorEnergy_3_2 = scaleE;
						ConfigBuff.ScaleFactorPSD_3_2 = scaleP;
						ConfigBuff.OffsetEnergy_3_2 = offsetE;
						ConfigBuff.OffsetPSD_3_2 = offsetP;
						break;
					case 3:
						//module 3
						ConfigBuff.ScaleFactorEnergy_4_2 = scaleE;
						ConfigBuff.ScaleFactorPSD_4_2 = scaleP;
						ConfigBuff.OffsetEnergy_4_2 = offsetE;
						ConfigBuff.OffsetPSD_4_2 = offsetP;
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
} */

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
int SetHighVoltage(XIicPs * Iic, unsigned char PmtId, int Value)
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
