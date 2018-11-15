/*
 * LI2C_Interface.c
 *
 *  Created on: Mar 16, 2017
 *      Author: GStoddard
 */

#include "LI2C_Interface.h"

int IicPsInit(u16 DeviceId)
{
	XIicPs_Config *Config;
	int iStatus = 0;

	/*
	* Initialize the IIC driver so that it's ready to use
	* Look up the configuration in the config table,
	* then initialize it.
	*/
	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config)
		iStatus = XST_FAILURE;

	iStatus = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (iStatus != XST_SUCCESS)
		iStatus = XST_FAILURE;

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	iStatus = XIicPs_SelfTest(&Iic);
	if (iStatus != XST_SUCCESS)
		iStatus = XST_FAILURE;

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	return iStatus;
}

/*****************************************************************************/
/**
*
* This function sends data and expects to receive data from slave as modular
* of 64.
*
* This function uses interrupt-driven mode of the device.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/
int IicPsMasterSend(u16 DeviceId, u8 * ptr_Send_Buffer, u8 * ptr_Recv_Buffer, int * iI2C_slave_addr)
{
	int iStatus = 0;
	XIicPs_Config *Config;

	/*
	* Initialize the IIC driver so that it's ready to use
	* Look up the configuration in the config table,
	* then initialize it.
	*
	* Need to do this each time we address a new device,
	*  so we'll just do it each time.
	*/
	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config)
		iStatus = XST_FAILURE;

	iStatus = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (iStatus != XST_SUCCESS)
		iStatus = XST_FAILURE;
	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	iStatus = XIicPs_SelfTest(&Iic);
	if (iStatus != XST_SUCCESS)
		iStatus = XST_FAILURE;

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	/*
	 * Send the buffer using the IIC and ignore the number of bytes sent
	 * as the return value since we are using it in interrupt mode.
	 */

	 XIicPs_MasterSend(&Iic, ptr_Send_Buffer, TEST_BUFFER_SIZE, *iI2C_slave_addr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&Iic))
	{
		/* NOP */
		//Will need to handle this timing us out
		//For example, if this becomes disconnected, the system will
		// wait here indefinitely because the I2C will never not
		// be busy. Thus, hanging the system.

		//need to set something to report failure if we time out
	}

	return XST_SUCCESS;
}

int IicPsMasterRecieve(u8 * ptr_Recv_Buffer, int * iI2C_slave_addr)
{
	int iStatus = 0;
	iStatus = XIicPs_MasterRecvPolled(&Iic, ptr_Recv_Buffer, 0x2, *iI2C_slave_addr);
	if (iStatus != XST_SUCCESS)
		iStatus = XST_FAILURE;

	return iStatus;
}
