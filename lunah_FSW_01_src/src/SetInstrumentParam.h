/*
 * SetInstrumentParam.h
 *
 *  Created on: Jun 20, 2018
 *      Author: IRDLAB
 */

/*
 * This file handles all interaction with the system parameters and the configuration file.
 */

#ifndef SRC_SETINSTRUMENTPARAM_H_
#define SRC_SETINSTRUMENTPARAM_H_

#include <stdio.h>
#include <xparameters.h>
#include "ff.h"
#include "lunah_defines.h"
#include "lunah_utils.h"
#include "LI2C_Interface.h"

/*
 * Mini-NS Configuration Parameter Structure
 * This is the collection of all of the Mini-NS system parameters.
 * Unless the user explicitly changes these, then the default will be filled in
 *  when the system boots. The configuration file is where the defaults are stored.
 * Each time that a parameter is changed by the user, that value is written to the
 *  configuration file, as well as to the current struct holding the parameters.
 * In this fashion, we are able to hold onto any changes that are made. This should
 *  reduce the amount of interaction necessary.
 *
 * See the Mini-NS ICD for a breakdown of these parameters and how to change them.
 * Current ICD version: 9.3.0
 *
 */
typedef struct {
	float ECalSlope;
	float EcalIntercept;
	int TriggerThreshold;
	int IntegrationBaseline;
	int IntegrationShort;
	int IntegrationLong;
	int IntegrationFull;
	int HighVoltageValue[4];
	float ScaleFactorEnergy_1_1;
	float ScaleFactorEnergy_1_2;
	float ScaleFactorEnergy_2_1;
	float ScaleFactorEnergy_2_2;
	float ScaleFactorEnergy_3_1;
	float ScaleFactorEnergy_3_2;
	float ScaleFactorEnergy_4_1;
	float ScaleFactorEnergy_4_2;
	float ScaleFactorPSD_1_1;
	float ScaleFactorPSD_1_2;
	float ScaleFactorPSD_2_1;
	float ScaleFactorPSD_2_2;
	float ScaleFactorPSD_3_1;
	float ScaleFactorPSD_3_2;
	float ScaleFactorPSD_4_1;
	float ScaleFactorPSD_4_2;
	float OffsetEnergy_1_1;
	float OffsetEnergy_1_2;
	float OffsetEnergy_2_1;
	float OffsetEnergy_2_2;
	float OffsetEnergy_3_1;
	float OffsetEnergy_3_2;
	float OffsetEnergy_4_1;
	float OffsetEnergy_4_2;
	float OffsetPSD_1_1;
	float OffsetPSD_1_2;
	float OffsetPSD_2_1;
	float OffsetPSD_2_2;
	float OffsetPSD_3_1;
	float OffsetPSD_3_2;
	float OffsetPSD_4_1;
	float OffsetPSD_4_2;
} CONFIG_STRUCT_TYPE;

// prototypes
void CreateDefaultConfig( void );
int InitConfig( void );
int SaveConfig( void );
int SetTriggerThreshold(int iTrigThreshold);
int SetNeutronCutGates(int moduleID, int ellipseNum, float ECut1, float ECut2, float PCut1, float PCut2);
//int SetWideNeutronCutGates(int moduleID, int ellipseNum, float WideECut1, float WideECut2, float WidePCut1, float WidePCut2);
int SetHighVoltage(XIicPs * Iic, unsigned char PmtId, int value);
int SetIntergrationTime(int Baseline, int Short, int Long, int Full);
int SetEnergyCalParam(float Slope, float Intercept);

#endif /* SRC_SETINSTRUMENTPARAM_H_ */
