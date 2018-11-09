/*
 * SetInstrumentParam.h
 *
 *  Created on: Jun 20, 2018
 *      Author: IRDLAB
 */

#ifndef SRC_SETINSTRUMENTPARAM_H_
#define SRC_SETINSTRUMENTPARAM_H_

// prototypes
int SetTriggerThreshold(int iTrigThreshold);
int SetNeutronCutGates(float ECut1, float ECut2, float PCut1, float PCut2);
int SetWideNeutronCutGates(float WideECut1, float WideECut2, float WidePCut1, float WidePCut2);
int SetHighVoltage(int PmtId, int value);
int SetIntergrationTime(int Baseline, int Short, int Long, int Full);
int SetEnergyCalParam(float Slope, float Intercept);

#endif /* SRC_SETINSTRUMENTPARAM_H_ */
