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
static char cConfigFile[] = "0:/MNSCONF.bin";
static CONFIG_STRUCT_TYPE ConfigBuff;
static int m_trigger_threshold;
static int m_baseline_integration_samples;
static int m_short_integration_samples;
static int m_long_integration_samples;
static int m_full_integration_samples;

//Box neutron cut parameters (these must be updated each time the system power cycles, they are not tracked)
//TODO: remove these when moving to the ellipse cuts
static CPS_BOX_CUTS_STRUCT_TYPE cps_box_cuts;

CPS_BOX_CUTS_STRUCT_TYPE * GetCPSCutVals( void )
{
	return &cps_box_cuts;
}

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
		.ECalIntercept=0.0,
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

/*
 * Allow a user to have access to the config buffer, so they can write it to a file/etc
 *
 * @param	None
 *
 * @return	Pointer to the config buffer
 */
CONFIG_STRUCT_TYPE * GetConfigBuffer( void )
{
	return &ConfigBuff;
}

/*
 * Access the integration times which are set with the system at the moment.
 * This function returns the value of the requested integration time in SAMPLES, not nanoseconds.
 *  The reason to return samples over ns is so that we don't have to convert to
 *  process the data.
 *
 * @param	None
 *
 * @return	(int)baseline samples
 */
int GetBaselineInt( void )
{
	return m_baseline_integration_samples;
}

int GetShortInt( void )
{
	return m_short_integration_samples;
}

int GetLongInt( void )
{
	return m_long_integration_samples;
}

int GetFullInt( void )
{
	return m_full_integration_samples;
}

/* This function handles initializing the system with the values from the config file.
 * If no config file exists, one will be created using the default (hard coded) values
 *  available to the system.
 * If the config file is found on the SD card, then it is read in and values are assigned
 *  to a config structure, but not set in the system.
 *
 * @param	None
 *
 * @return	FR_OK (0) or command FAILURE (!0)
 *
 */
int InitConfig( void )
{
	uint NumBytesWr;
	uint NumBytesRd;
	FRESULT fres = FR_OK;
	FIL ConfigFile;
	int RetVal = 0;
	int ConfigSize = sizeof(ConfigBuff);

	// check if the config file exists
	fres = f_open(&ConfigFile, cConfigFile, FA_READ|FA_WRITE|FA_OPEN_EXISTING);
	if( fres == FR_OK )
	{
		fres = f_lseek(&ConfigFile, 0);
		if(fres == FR_OK)
			fres = f_read(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesRd);
		f_close(&ConfigFile);
	}
	else if(fres == FR_NO_FILE)// The config file does not exist, create it
	{
		CreateDefaultConfig();
		fres = f_open(&ConfigFile, cConfigFile, FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
		if(fres == FR_OK)
			fres = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
		f_close(&ConfigFile);
	}
	else
	{
		//TODO: handle an error which is not OK or No File

	}

	RetVal = (int)fres;
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
	FIL ConfigFile;
	int RetVal = 0;
	int ConfigSize = sizeof(ConfigBuff);

	//we assume the config file exists already
	F_RetVal = f_open(&ConfigFile, cConfigFile, FA_READ|FA_WRITE|FA_OPEN_ALWAYS);
	if(F_RetVal == FR_OK)
		F_RetVal = f_lseek(&ConfigFile, 0);
	if(F_RetVal == FR_OK)
		F_RetVal = f_write(&ConfigFile, &ConfigBuff, ConfigSize, &NumBytesWr);
	//close regardless of the return value
	F_RetVal = f_close(&ConfigFile);

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
	if((iTrigThreshold > 0) && (iTrigThreshold < 16384))
	{
		//set the threshold in the FPGA
		Xil_Out32(XPAR_AXI_GPIO_10_BASEADDR, (u32)(iTrigThreshold));
		//read back value from the FPGA and compare with intended change
		if(iTrigThreshold == Xil_In32(XPAR_AXI_GPIO_10_BASEADDR))
		{
			ConfigBuff.TriggerThreshold = iTrigThreshold;
			SaveConfig();
			m_trigger_threshold = iTrigThreshold;
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
			ConfigBuff.ECalIntercept = Intercept;
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
 * 					Valid input range: 1 - 4
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
	int status = CMD_SUCCESS;

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
	case 0: //BOX CUTS //TODO: remove this case when updating to ellipse cuts
		switch(ellipseNum)
		{
		case 0:
			cps_box_cuts.ecut_low = scaleE;
			cps_box_cuts.ecut_high = scaleP;
			cps_box_cuts.pcut_low = offsetE;
			cps_box_cuts.pcut_high = offsetP;
			//set variable to indicate that we should use these values and not the defined ones
			cps_box_cuts.set_cuts = 1;
			break;
		case 1:
			cps_box_cuts.ecut_wide_low = scaleE;
			cps_box_cuts.ecut_wide_high = scaleP;
			cps_box_cuts.pcut_wide_low = offsetE;
			cps_box_cuts.pcut_wide_high = offsetP;
			//set variable to indicate that we should use these values and not the defined ones
			cps_box_cuts.set_cuts = 1;
			break;
		default:
			cps_box_cuts.set_cuts = 0;
			status = CMD_FAILURE;
			break;
		}
		break;
	case 1:	//module 1
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
	case 2:	//module 2
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
	case 3:	//module 3
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
	case 4:	//module 4
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
		//just leave the cuts, no change
		status = CMD_FAILURE;
		break;
	}
	if(status == CMD_SUCCESS)
		SaveConfig();

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
int SetIntegrationTime(int Baseline, int Short, int Long, int Full)
{
	int status = 0;

	//should do more error checking on the range for baseline and full
	//TODO: Get the range of acceptable values for the integration times
	if((Baseline >= -200) && (Full < 16384))
	{
		if((Baseline < Short) && ( Short < Long) && (Long < Full))
		{
			Xil_Out32 (XPAR_AXI_GPIO_0_BASEADDR, ((u32)(Baseline+52)/4));
			Xil_Out32 (XPAR_AXI_GPIO_1_BASEADDR, ((u32)(Short+52)/4));
			Xil_Out32 (XPAR_AXI_GPIO_2_BASEADDR, ((u32)(Long+52)/4));
			Xil_Out32 (XPAR_AXI_GPIO_3_BASEADDR, ((u32)(Full+52)/4));

			//error check to make sure that we have set all the values correctly
			//we can worry about error checking later on in the FSW process
			//read back the values from the FPGA //They should be equal to the values we set
			if(Xil_In32(XPAR_AXI_GPIO_0_BASEADDR) == (Baseline+52)/4)
				if(Xil_In32(XPAR_AXI_GPIO_1_BASEADDR) == (Short+52)/4)
					if(Xil_In32(XPAR_AXI_GPIO_2_BASEADDR) == (Long+52)/4)
						if(Xil_In32(XPAR_AXI_GPIO_3_BASEADDR) == (Full+52)/4)
						{
							ConfigBuff.IntegrationBaseline = Baseline;
							ConfigBuff.IntegrationShort = Short;
							ConfigBuff.IntegrationLong = Long;
							ConfigBuff.IntegrationFull = Full;
							SaveConfig();
							m_baseline_integration_samples = (INTEG_TIME_START + Baseline) / NS_TO_SAMPLES + 1;	//add one sample to each
							m_short_integration_samples = (INTEG_TIME_START + Short) / NS_TO_SAMPLES + 1;
							m_long_integration_samples = (INTEG_TIME_START + Long) / NS_TO_SAMPLES + 1;
							m_full_integration_samples = (INTEG_TIME_START + Full) / NS_TO_SAMPLES + 1;
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

			//if this comes back as CMD_FAILURE, try and set the default value?
		}
		else
			status = CMD_FAILURE;
	}
	else
		status = CMD_FAILURE;

	return status;
}


int ApplyDAQConfig( XIicPs * Iic )
{
	int status = CMD_SUCCESS;

	status = SetEnergyCalParam(ConfigBuff.ECalSlope, ConfigBuff.ECalIntercept);
	if(status == CMD_SUCCESS)
		status = SetTriggerThreshold(ConfigBuff.TriggerThreshold);
	if(status == CMD_SUCCESS)
		status = SetIntegrationTime(ConfigBuff.IntegrationBaseline, ConfigBuff.IntegrationShort, ConfigBuff.IntegrationLong, ConfigBuff.IntegrationFull);
	if(status == CMD_SUCCESS)
		status = SetHighVoltage(Iic, 1, ConfigBuff.HighVoltageValue[0]);
	if(status == CMD_SUCCESS)
		status = SetHighVoltage(Iic, 2, ConfigBuff.HighVoltageValue[1]);
	if(status == CMD_SUCCESS)
		status = SetHighVoltage(Iic, 3, ConfigBuff.HighVoltageValue[2]);
	if(status == CMD_SUCCESS)
		status = SetHighVoltage(Iic, 4, ConfigBuff.HighVoltageValue[3]);
	//set n cuts
	if(status == CMD_SUCCESS)
		status = SetNeutronCutGates(1, 1, ConfigBuff.ScaleFactorEnergy_1_1, ConfigBuff.ScaleFactorPSD_1_1, ConfigBuff.OffsetEnergy_1_1, ConfigBuff.OffsetPSD_1_1);
	if(status == CMD_SUCCESS)
		status = SetNeutronCutGates(1, 2, ConfigBuff.ScaleFactorEnergy_1_2, ConfigBuff.ScaleFactorPSD_1_2, ConfigBuff.OffsetEnergy_1_2, ConfigBuff.OffsetPSD_1_2);
	if(status == CMD_SUCCESS)
		status = SetNeutronCutGates(2, 1, ConfigBuff.ScaleFactorEnergy_2_1, ConfigBuff.ScaleFactorPSD_2_1, ConfigBuff.OffsetEnergy_2_1, ConfigBuff.OffsetPSD_2_1);
	if(status == CMD_SUCCESS)
		status = SetNeutronCutGates(2, 2, ConfigBuff.ScaleFactorEnergy_2_2, ConfigBuff.ScaleFactorPSD_2_2, ConfigBuff.OffsetEnergy_2_2, ConfigBuff.OffsetPSD_2_2);
	if(status == CMD_SUCCESS)
		status = SetNeutronCutGates(3, 1, ConfigBuff.ScaleFactorEnergy_3_1, ConfigBuff.ScaleFactorPSD_3_1, ConfigBuff.OffsetEnergy_3_1, ConfigBuff.OffsetPSD_3_1);
	if(status == CMD_SUCCESS)
		status = SetNeutronCutGates(3, 2, ConfigBuff.ScaleFactorEnergy_3_2, ConfigBuff.ScaleFactorPSD_3_2, ConfigBuff.OffsetEnergy_3_2, ConfigBuff.OffsetPSD_3_2);
	if(status == CMD_SUCCESS)
		status = SetNeutronCutGates(4, 1, ConfigBuff.ScaleFactorEnergy_4_1, ConfigBuff.ScaleFactorPSD_4_1, ConfigBuff.OffsetEnergy_4_1, ConfigBuff.OffsetPSD_4_1);
	if(status == CMD_SUCCESS)
		status = SetNeutronCutGates(4, 2, ConfigBuff.ScaleFactorEnergy_4_2, ConfigBuff.ScaleFactorPSD_4_2, ConfigBuff.OffsetEnergy_4_2, ConfigBuff.OffsetPSD_4_2);

	//TODO: error check

	return status;
}

