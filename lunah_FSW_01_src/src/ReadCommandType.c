/*
 * ReadCommandType.c
 *
 *  Created on: Jul 11, 2017
 *      Author: GStoddard
 */
//////////////////////////// ReadCommandType////////////////////////////////
// This function takes in the RecvBuffer read in by readcommandpoll() and
// determines what the input was and returns an int to main which indicates
// that value. It also places the 'leftover' part of the buffer into a separate
// buffer with just that number(s)/filename/other for ease of use.

#include "ReadCommandType.h"

//used by LApp
int iPollBufferIndex;

int ReadCommandType(char * RecvBuffer, XUartPs *Uart_PS) {
	//Variables
	char is_line_ending = '0';
	int ret = 0;
	int detectorVal = 0;
	int commandNum = 999;	//this value tells the main menu what command we read from the rs422 buffer
	int firstVal = 0;
	int secondVal = 0;
	int thirdVal = 0;
	int fourthVal = 0;
	float ffirstVal = 0.0;
	float fsecondVal = 0.0;
	float fthirdVal = 0.0;
	float ffourthVal = 0.0;
	unsigned long long int realTime = 0;

	char commandBuffer[20] = "";
	char commandBuffer2[50] = "";

	iPollBufferIndex += XUartPs_Recv(Uart_PS, (u8 *)RecvBuffer + iPollBufferIndex, 100 - iPollBufferIndex);	//pollbuffindex holds the number of bytes read from the user
	if(iPollBufferIndex > 99)	//read too much
		return 100;
	if(iPollBufferIndex != 0)
	{
		is_line_ending = RecvBuffer[iPollBufferIndex - 1];	//checks whether the last character is a line ending
		if((is_line_ending == '\n') || (is_line_ending == '\r'))
		{
			ret = sscanf(RecvBuffer, " %[^_]", commandBuffer);	// copy the command (everything before the underscore)

			if(!strcmp(commandBuffer, "DAQ"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d", &detectorVal, &firstVal);	//check that there is one int after the underscore // may need a larger value here to take a 64-bit time

				if(ret != 2)	//invalid input
					commandNum = -1;
				else			//proper input
					commandNum = 0;

			}
			else if(!strcmp(commandBuffer, "WF"))
			{
				xil_printf("WF received");
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d", &detectorVal, &firstVal);	//check for the _number of the waveform

				if(ret != 2)	//invalid input
					commandNum = -1;
				else
					commandNum = 1;
			}
			else if(!strcmp(commandBuffer, "TMP"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d", &detectorVal);	//check for the _number of the waveform

				if(ret != 1)	//invalid input
					commandNum = -1;
				else
					commandNum = 2;
			}
			else if(!strcmp(commandBuffer, "GETSTAT"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d", &detectorVal);

				if(ret != 1)	//invalid input
					commandNum = -1;
				else
					commandNum = 3;
			}
			else if(!strcmp(commandBuffer, "DISABLE"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %s_%d", commandBuffer2,&detectorVal);	//check for the _number of the waveform

				if(ret != 1)	//invalid input
				{
					//xil_printf("\n\rInvalid input; wf_number not recognized.\n\r");
					commandNum = -1;
				}
				else
				{
					if(!strcmp(commandBuffer2, "ACT"))
						commandNum = 4;
					else if(!strcmp(commandBuffer2, "TEC"))
						commandNum = 5;
					else
						commandNum = -1;
				}
			}
			else if(!strcmp(commandBuffer, "ENABLE"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %s", commandBuffer2);

				if(ret != 1)	//invalid input
				{
					commandNum = -1;
				}
				else	//good input so far
				{
					if(!strcmp(commandBuffer2, "TEC"))	//proper input
						commandNum = 6;
					else
						commandNum = -1;	//anything else
				}
			}
			else if(!strcmp(commandBuffer, "TX"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %s", commandBuffer2);

				if(ret != 1)
					commandNum = -1;
				else
					commandNum = 7;
			}
			else if(!strcmp(commandBuffer, "DEL"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %s", commandBuffer2);

				if(ret != 1)
					commandNum = -1;
				else
					commandNum = 8;
			}
			else if(!strcmp(commandBuffer, "LS"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %s", commandBuffer2);
				if(ret != 1)
					commandNum = -1;
				else
					commandNum = 9;
			}
			else if(!strcmp(commandBuffer, "TRG"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d", &firstVal);	//check that there is one int after the underscore

				if(ret != 1)	//invalid input
				{
					//xil_printf("Invalid input; wrong number of thresholds.\n\r");
					commandNum = -1;
				}
				else
					commandNum = 10;
			}
			else if(!strcmp(commandBuffer, "NGATES"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %f_%f_%f_%f", &ffirstVal, &fsecondVal, &fthirdVal, &ffourthVal);	//check for the _number of the waveform

				if(ret != 4)	//invalid input
				{
					//xil_printf("Invalid input; wf_number not recognized.\n\r");
					commandNum = -1;
				}
				else
					commandNum = 11;
			}
			else if(!strcmp(commandBuffer, "HV"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d", &firstVal, &secondVal);	//check for the _number of the waveform

				if(ret != 2)	//invalid input
				{
					//xil_printf("Invalid input; _temperature_timeout not recognized.\n\r");
					commandNum = -1;
				}
				else
					commandNum = 12;
			}
			else if(!strcmp(commandBuffer, "INT"))
			{
				//copy over the integrals
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d_%d_%d", &firstVal, &secondVal, &thirdVal, &fourthVal);

				//check the result and see if input from the user was correct
				if(ret < 4 && ret > -1)
				{
					//not enough numbers given
					//xil_printf("Invalid input, too few integration times given.\n\r");
					commandNum = -1;
				}
				else if((ret <= -1) || (ret > 4))
				{
					//invalid input, return a message indicating failure and return to main menu
					//too many ints or fail to read
					//xil_printf("Invalid input, too many or no integration times given.\n\r");
					commandNum = -1;
				}
				else
					commandNum = 13;
			}
			else if(!strcmp(commandBuffer, "ECAL"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %f_%f", &ffirstVal, &fsecondVal);	//check for the _number of the waveform

				if(ret != 2)	//invalid input
					commandNum = -1;
				else
					commandNum = 14;
			}
			else if(!strcmp(commandBuffer, "BREAK"))
				commandNum = 15;
			else if(!strcmp(commandBuffer, "START"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %llud", &realTime);	//check for the _number of the waveform

				if(ret != 1)	//invalid input
					commandNum = -1;
				else
					commandNum = 16;
			}
			else if(!strcmp(commandBuffer, "END"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %llud", &realTime);	//check for the _number of the waveform

				if(ret != 1)	//invalid input
					commandNum = -1;
				else
					commandNum = 17;
			}
			else if(!strcmp(commandBuffer, "ENDTMP"))
				commandNum = 18;
			else
			{
				commandNum = -1;	// a -1 indicates failure, a positive value indicates success
				memset(RecvBuffer, '0', 100);
			}

			//now check to see if the command pertains to this detector
			if(detectorVal == 0)	//this detector
				detectorVal += 0;	//will pass on the command
			else if(detectorVal ==1)	//the other detector
				detectorVal += 900;		//will fail the command at the switch

		}
	}

	return commandNum;	// If we don't find a return character, don't try and check the commandBuffer for one
}

int PollUart(char * RecvBuffer, XUartPs *Uart_PS)
{
	//Variable definitions
	int returnVal = 0;
	char commandBuffer[20] = "";

	iPollBufferIndex += XUartPs_Recv(Uart_PS, (u8 *)RecvBuffer + iPollBufferIndex, 100 - iPollBufferIndex);
	if(RecvBuffer[iPollBufferIndex-1] == '\n' || RecvBuffer[iPollBufferIndex-1] == '\r')	//if we find an 'enter'...
	{
		sscanf(RecvBuffer, " %[^_]", commandBuffer);	//copy the command (everything before the underscore)
		if(!strcmp(commandBuffer, "BREAK"))					// and check it against all the commands that are acceptable
			returnVal = 14;
		else if(!strcmp(commandBuffer, "START"))
			returnVal = 15;
		else if(!strcmp(commandBuffer, "END"))
			returnVal = 16;
		else if(!strcmp(commandBuffer, "ENDTMP"))
			returnVal = 17;
		else	//input was bad and is getting thrown out
		{
			returnVal = 18;
			memset(RecvBuffer, '0', 100);
		}
	}

	return returnVal;
}
