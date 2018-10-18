/*
 * lunah_defines.h
 *
 *  Created on: Jun 20, 2018
 *      Author: IRDLAB
 */

#include "xparameters.h"

#ifndef SRC_LUNAH_DEFINES_H_
#define SRC_LUNAH_DEFINES_H_

#define BREAKUP_MAIN TRUE

#define LOG_FILE_BUFF_SIZE	120
#define UART_DEVICEID		XPAR_XUARTPS_0_DEVICE_ID
#define SW_BREAK_GPIO		51
#define IIC_DEVICE_ID_0		XPAR_XIICPS_0_DEVICE_ID	//sensor head
#define IIC_DEVICE_ID_1		XPAR_XIICPS_1_DEVICE_ID	//thermometer/pot on digital board
#define FILENAME_SIZE		50
#define	TEC_PIN				18
#define DATA_PACKET_SIZE	2040
#define PAYLOAD_MAX_SIZE	2028
#define DATA_BUFFER_SIZE	4096

#define TWODH_X_BINS		260
#define	TWODH_Y_BINS		30
#define SYNC_MARKER_SIZE	4
#define CHECKSUM_SIZE		4
#define CCSDS_HEADER_SIZE	11

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
