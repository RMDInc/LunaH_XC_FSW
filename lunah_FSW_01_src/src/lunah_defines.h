/*
 * lunah_defines.h
 *
 *  Created on: Jun 20, 2018
 *      Author: IRDLAB
 */

#include "xparameters.h"

#ifndef SRC_LUNAH_DEFINES_H_
#define SRC_LUNAH_DEFINES_H_

#define MNS_DETECTOR_NUM	0

#define LOG_FILE_BUFF_SIZE	120
#define UART_DEVICEID		XPAR_XUARTPS_0_DEVICE_ID
#define SW_BREAK_GPIO		51
#define IIC_DEVICE_ID_0		XPAR_XIICPS_0_DEVICE_ID	//sensor head
#define IIC_DEVICE_ID_1		XPAR_XIICPS_1_DEVICE_ID	//thermometer/pot on digital board
#define FILENAME_SIZE		50
#define	TEC_PIN				18
#define EVTS_EVENT_SIZE		8
#define SIZEOF_FILENAME		34 //filename example: "I000001_R000001_S000001_cps.bin"
#define DATA_PACKET_SIZE	2040
#define PAYLOAD_MAX_SIZE	2028
#define DATA_BUFFER_SIZE	4096
#define EVENT_BUFFER_SIZE	16384
#define SIZEOF_HEADER_TIMES	14
#define TWODH_X_BINS		260
#define	TWODH_Y_BINS		30
#define TWODH_ENERGY_MAX	1000000
#define TWODH_PSD_MAX		2.0
#define SYNC_MARKER_SIZE	4
#define CHECKSUM_SIZE		4
#define CCSDS_HEADER_SIZE	11

// Command definitions
#define DAQ_CMD			0
#define WF_CMD			1
#define READ_TMP_CMD	2
#define GETSTAT_CMD		3
#define DISABLE_ACT_CMD	4
#define ENABLE_ACT_CMD	5
#define TX_CMD			6
#define DEL_CMD			7
#define LS_CMD			8
#define TXLOG_CMD		9
#define	CONF_CMD		10
#define TRG_CMD			11
#define ECAL_CMD		12
#define NGATES_CMD		13
#define HV_CMD			14
#define INT_CMD			15
#define BREAK_CMD		16
#define START_CMD		17
#define END_CMD			18
#define INPUT_OVERFLOW	100

//Command SUCCESS/FAILURE values
#define CMD_FAILURE		0	// 0 == FALSE
#define CMD_SUCCESS		1	// non-zero == TRUE

//APID Packet Types
#define APID_CMD_SUCC	0
#define APID_CMD_FAIL	1
#define APID_SOH		2
#define APID_LS_FILES	3
#define APID_TEMP		4
#define APID_MNS_CPS	5
#define APID_MNS_WAV	6
#define APID_MNS_EVTS	7
#define APID_MNS_2DH	8
#define APID_LOG_FILE	9
#define APID_CONFIG		10

//MNS DATA PRODUCT TYPES
#define DATA_TYPE_EVTS 	0
#define DATA_TYPE_WAV	1
#define DATA_TYPE_2DH	2
#define DATA_TYPE_CPS	3

//DAQ FINAL STATE
#define DAQ_BREAK		0
#define DAQ_TIME_OUT	1
#define DAQ_END			2

enum LoopStateTypes
{
    MainLoopState,
	DataQState,
	WaitStartState,
	CollectDataState,
};

enum ReadTempState
{
   AlogTempCmdSent,
   AlogTempCmdRcved,
   DigTempCmdSent,
   DigTempCmdRcved,
   ModTempCmdSent,
   ModTempCmdRcved,
};

int DataAcqInit(int Command, int orbit_number);

#endif /* SRC_LUNAH_DEFINES_H_ */
