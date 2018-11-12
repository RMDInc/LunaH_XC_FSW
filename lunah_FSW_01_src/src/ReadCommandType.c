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

//STATE VARIABLES
static int iPollBufferIndex = 0;
//last command holder buff for giving this to the data packets
static char last_command[50] = "";
//last command size holder
static int last_command_size = 0;
//Retain the scanned command parameters so they may be accessed later
static int firstVal = 0;
static int secondVal = 0;
static int thirdVal = 0;
static int fourthVal = 0;
static float ffirstVal = 0.0;
static float fsecondVal = 0.0;
static float fthirdVal = 0.0;
static float ffourthVal = 0.0;

/* Delete a scanned out command from the buffer then shift the buffer over */
// This function deletes bytes from the beginning of a char buffer then
// shifts the rest of the contents over by that amount
// @param	A pointer to the char buffer to shift
// @param	The number of bytes to shift by
// @param	The number of bytes in the string left in the buffer
//			This is the total minus the bytes we want to delete
void bufferShift(char * buff, int bytes_to_del, int buff_strlen)
{
	//check on the values
	if(bytes_to_del < 0){
		xil_printf("error 1 in bufferShif\r\n");
		xil_printf("buffer not cleared\r\n");
	}
	else if(buff_strlen < 0){
		xil_printf("error 2 in bufferShift\r\n");
		xil_printf("buffer not cleared\r\n");
	}
	else{
		//delete the values in the buffer
		memset(buff, '\0', bytes_to_del);

		//need to figure out how long exactly it takes for memset to safely finish
		usleep(50);	//wait for memset to finish?

		//shift over the rest of the commands
		memmove(buff, buff+bytes_to_del, buff_strlen);
		//zero out the memory above what is left in the buffer
		memset(buff+buff_strlen, '\0', 100-buff_strlen);

		usleep(50);
	}

	return;
}

/* Getter function to access the previous command entered into the buffer */
//This function accesses the last_command buffer which holds the
// previous command scanned by the system.
//We do not want to return the '\n' from the command
char * GetLastCommand( void )
{
	return last_command;
}

/* Getter to access the size of the previous command entered into the buffer */
//This function accesses the last_command buffer which holds the
// previous command scanned by the system
//
//As a note, need to save the size of the command because
// the sizeof(last_command) call will just return 50 bytes, as this is the size of
// the buffer as a whole.
unsigned int GetLastCommandSize( void )
{
	return last_command_size;
}

int ReadCommandType(char * RecvBuffer, XUartPs *Uart_PS) {
	//Variables
	//char is_line_ending = '\0';
	int ret = 0;
	int bytes_scanned = 0;
	int detectorVal = 0;
	int commandNum = 999;	//this value tells the main menu what command we read from the rs422 buffer
	unsigned long long int realTime = 0;

	char commandBuffer[20] = "";
	char commandBuffer2[50] = "";

	iPollBufferIndex += XUartPs_Recv(Uart_PS, (u8 *)RecvBuffer + iPollBufferIndex, 100 - iPollBufferIndex);	//pollbuffindex holds the number of bytes read from the user
	if(iPollBufferIndex > 99)	//read too much
		return 100;
	if(iPollBufferIndex != 0)
	{
		//checks whether the last character is a line ending
		if((RecvBuffer[iPollBufferIndex - 1] == '\n') || (RecvBuffer[iPollBufferIndex - 1] == '\r'))
		{
			ret = sscanf(RecvBuffer, " %[^_]", commandBuffer);	// copy the command (everything before the underscore)
			if(ret == -1)
			{
				//something in the buffer and the last char is a newline
				//but we couldn't scan it properly
				commandNum = -1;
			}
			else if(!strcmp(commandBuffer, "DAQ"))
			{
				//check that there is one int after the underscore // may need a larger value here to take a 64-bit time
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d", &detectorVal, &firstVal);

				if(ret != 2)	//invalid input
					commandNum = -1;
				else			//proper input
					commandNum = 0;

			}
			else if(!strcmp(commandBuffer, "WF"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d", &detectorVal, &firstVal);

				if(ret != 2)	//invalid input
					commandNum = -1;
				else
					commandNum = 1;
			}
			else if(!strcmp(commandBuffer, "READTEMP"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d", &detectorVal);

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
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %[^_]_%d", commandBuffer2,&detectorVal);

				if(ret != 2)	//invalid input
				{
					commandNum = -1;
				}
				else	//scanned two items from the buffer
				{
					if(!strcmp(commandBuffer2, "ACT"))
						commandNum = 4;
					else
						commandNum = -1;
				}
			}
			else if(!strcmp(commandBuffer, "ENABLE"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %[^_]_%d", commandBuffer2, &detectorVal);

				if(ret != 2)	//invalid input
				{
					commandNum = -1;
				}
				else	//scanned two items from the buffer
				{
					if(!strcmp(commandBuffer2, "ACT"))	//proper input
						commandNum = 5;
					else
						commandNum = -1;	//anything else
				}
			}
			else if(!strcmp(commandBuffer, "TX"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%s", &detectorVal, commandBuffer2);

				if(ret != 2)
					commandNum = -1;
				else
					commandNum = 6;
			}
			else if(!strcmp(commandBuffer, "DEL"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%s", &detectorVal, commandBuffer2);

				if(ret != 2)
					commandNum = -1;
				else
					commandNum = 7;
			}
			else if(!strcmp(commandBuffer, "LS"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %s_%d", commandBuffer2, &detectorVal);

				if(ret != 2)	//invalid input
					commandNum = -1;
				else	//scanned two items from the buffer
				{
					if(!strcmp(commandBuffer2, "FILES"))	//proper input
						commandNum = 8;
					else
						commandNum = -1;	//anything else
				}
			}
			else if(!strcmp(commandBuffer, "TXLOG"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d", &detectorVal);
				if(ret != 1)
					commandNum = -1;
				else
					commandNum = 9;
			}
			else if(!strcmp(commandBuffer, "CONF"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d", &detectorVal);
				if(ret != 1)
					commandNum = -1;
				else
					commandNum = 10;
			}
			else if(!strcmp(commandBuffer, "TRG"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d", &detectorVal, &firstVal);

				if(ret != 2)	//invalid input
					commandNum = -1;
				else
					commandNum = 11;
			}
			else if(!strcmp(commandBuffer, "ECAL"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%f_%f", &detectorVal, &ffirstVal, &fsecondVal);

				if(ret != 3)	//invalid input
					commandNum = -1;
				else
					commandNum = 12;
			}
			else if(!strcmp(commandBuffer, "NGATES"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%f_%f_%f_%f", &detectorVal, &ffirstVal, &fsecondVal, &fthirdVal, &ffourthVal);

				if(ret != 5)	//invalid input
					commandNum = -1;
				else
					commandNum = 13;
			}
			else if(!strcmp(commandBuffer, "NWGATES"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%f_%f_%f_%f", &detectorVal, &ffirstVal, &fsecondVal, &fthirdVal, &ffourthVal);
				if(ret != 5)
					commandNum = -1;
				else
					commandNum = 14;
			}
			else if(!strcmp(commandBuffer, "HV"))
			{
				//check for the _number of the waveform
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d_%d", &detectorVal, &firstVal, &secondVal);

				if(ret != 3)	//invalid input
					commandNum = -1;
				else
					commandNum = 15;
			}
			else if(!strcmp(commandBuffer, "INT"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%d_%d_%d_%d", &detectorVal, &firstVal, &secondVal, &thirdVal, &fourthVal);

				if( ret != 5 )
					commandNum = -1;	//bad input, nothing scanned
				else
					commandNum = 16;
			}
			else if(!strcmp(commandBuffer, "BREAK"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d", &detectorVal);
				if(ret != 1)
					commandNum = -1;
				else
					commandNum = 17;
			}
			else if(!strcmp(commandBuffer, "START"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%llud_%d", &detectorVal, &realTime, &firstVal);	//check for the _number of the waveform

				if(ret != 3)	//invalid input
					commandNum = -1;
				else
					commandNum = 18;
			}
			else if(!strcmp(commandBuffer, "END"))
			{
				ret = sscanf(RecvBuffer + strlen(commandBuffer) + 1, " %d_%llud", &detectorVal, &realTime);	//check for the _number of the waveform

				if(ret != 2)	//invalid input
					commandNum = -1;
				else
					commandNum = 19;
			}
			else
			{
				//there was something in the buffer and we scanned it
				// but it did not match any of the commands above
				//Reject this command
				commandNum = -1;

			}

			//now check to see if the command pertains to this detector
			if(detectorVal == 0)	//this detector
				commandNum += 0;	//will pass on the command
			else if(detectorVal ==1)	//the other detector
				commandNum += 900;		//will fail the command at the switch

		}//end of is_line_ending
	}//end of ifpoll != 0

	//if the value of command num is such that we read out a command
	//whether right or wrong, we want to get rid of it
	//shift off the command that we have read in now that it is characterized
	if(commandNum != 999)
	{
		//get the number of bytes that were part of the command:
		ret = sscanf(RecvBuffer, " %s\n%n", last_command, &bytes_scanned);
		//records the size of the full command that we just read in (including parameters)
		last_command_size = bytes_scanned;
		if(ret == -1)	//indicates that no data could be successfully interpreted
		{
			//catch the error
			if(iPollBufferIndex == 1)
			{
				//the buffer should only have one byte in it
				//set that byte to null and set the buffer index back
				// but since we ignore whitespace, just set the first byte to null (\0)
				memset(RecvBuffer, '\0', 1);
				//we took a character out and now there should be nothing in the buffer
				iPollBufferIndex -= 1;
			}
			else
			{
				//there is more than just one byte in the recvbuffer, we need
				// to read through them one-by-one
				bufferShift(RecvBuffer, 1, iPollBufferIndex);
				//we removed one byte from the buffer
				iPollBufferIndex -= 1;
			}
		}
		else	//proceed as normal
		{
			//we want to remove some bytes from the buffer, so move the buffer length
			iPollBufferIndex -= bytes_scanned;
			bufferShift(RecvBuffer, bytes_scanned, iPollBufferIndex);
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

/* Getter to access the parameters entered with a command */
//This function accesses the firstVal-fourthVal integers which are set after
// a commanded function with parameters is read in.
//
// @param	param_num	This is a number (1-4) which is where in the parameter
// 						set the value appeared.
//						eg. a full integration time is param_num = 4
//
// @return	(int) returns the value assigned when the command was scanned
int GetIntParam( int param_num )
{
	int value = 0;

	switch(param_num)
	{
	case 1:
		//get the first integer parameter
		value = firstVal;
		break;
	case 2:
		//get the first integer parameter
		value = secondVal;
		break;
	case 3:
		//get the first integer parameter
		value = thirdVal;
		break;
	case 4:
		//get the first integer parameter
		value = fourthVal;
		break;
	default:
		//if the param_num was weird
		//set a ridiculous number that we can detect as an error
		value = -999;
		break;
	}

	return value;
}

/* Getter to access the parameters entered with a command */
//This function accesses the ffirstVal-ffourthVal floats which are set after
// a commanded function with parameters is read in.
//
// @param	param_num	This is a number (1-4) which is where in the parameter
// 						set the value appeared.
//						eg. the PSD cut 2 is param_num = 4
//
// @return	(int) returns the value assigned when the command was scanned
float GetFloatParam( int param_num )
{
	float value = 0;

	switch(param_num)
	{
	case 1:
		//get the first integer parameter
		value = ffirstVal;
		break;
	case 2:
		//get the first integer parameter
		value = fsecondVal;
		break;
	case 3:
		//get the first integer parameter
		value = fthirdVal;
		break;
	case 4:
		//get the first integer parameter
		value = ffourthVal;
		break;
	default:
		//if the param_num was weird
		//set a ridiculous number that we can detect as an error
		value = -999.99;
		break;
	}

	return value;
}
