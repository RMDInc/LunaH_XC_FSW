/*
 * main.c
 *
 *  Created on: Sep 19, 2019
 *      Author: GStoddard
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */
#define CHECKSUM_SIZE		4
#define SYNC_MARKER_SIZE	4
#define RMD_CHECKSUM_SIZE	2
#define CCSDS_HEADER_DATA	7
#define CCSDS_HEADER_PRIM	10
#define CCSDS_HEADER_FULL	11

/* Loops over all the bytes in the packet and determines if the checksums match

	@param int * pointer to the beginning of the packet array
	@param int   the length of the data within the packet to loop over

	@return int 1 = simple match, 2 = fletcher match, 4 = bct match, 7 = all match, 0 = fail
*/
int verifyCRC( unsigned char *packet_array, int packet_length )
{
	unsigned short bct_checksum = 0;
	int match = 0;
	int iterator = 0;
	int total_packet_size = 0;
	int rmd_checksum_simple = 0;
	int rmd_checksum_Fletch = 0;
	unsigned int TX_rmd_simple = 0;
	unsigned int TX_rmd_Fletch = 0;
	unsigned int TX_bct_checks = 0;

	total_packet_size = packet_length + CCSDS_HEADER_FULL;	//includes both primary and secondary CCSDS headers
	TX_rmd_simple = packet_array[total_packet_size - CHECKSUM_SIZE];
	TX_rmd_Fletch = packet_array[total_packet_size - CHECKSUM_SIZE + 1];
	TX_bct_checks = (packet_array[total_packet_size - CHECKSUM_SIZE + 2] << 8) + packet_array[total_packet_size - CHECKSUM_SIZE + 3];

	//calculate the RMD checksums
	while(iterator <= (packet_length - CHECKSUM_SIZE))
	{
		rmd_checksum_simple = (rmd_checksum_simple + packet_array[CCSDS_HEADER_PRIM + iterator]) % 255;
		rmd_checksum_Fletch = (rmd_checksum_Fletch + rmd_checksum_simple) % 255;
		iterator++;
	}

	//calculate the BCT checksum
	iterator = 0;
	while(iterator < (packet_length - RMD_CHECKSUM_SIZE + CCSDS_HEADER_DATA))
	{
		bct_checksum += packet_array[SYNC_MARKER_SIZE + iterator];
		iterator++;
	}

	//check against accepted values
	if(rmd_checksum_simple == TX_rmd_simple)
		match |= 1;
	if(rmd_checksum_Fletch == TX_rmd_Fletch)
		match |= 2;
	if(bct_checksum == TX_bct_checks)
		match |= 4;

	return match;
}

int main(int argc, char *argv[]) {
	//random printf stuff
	setbuf(stdout, NULL);

	long m_file_size = 0;
	int m_bytes_read = 0;
	int m_bytes_written = 0;
	int i = 0;
	int m_iter = 0;
	int m_file_size_iter = 1;	//starts at 1
	int m_packet_number = 0;
	int m_detector_number = 0;
	int m_packet_apid = 0;
	int m_group_flags = 0;
	int m_sequence_count = 0;
	int m_packet_length = 0;
	int m_total_packet_length = 0;
	int m_header_bytes = 0;
	int m_simple_checksum = 0;
	int m_Fletcher_checksum = 0;
	int m_ccsds_checksum = 0;
	int m_checksum_match = 0;
	//packet specific data variables here
	//SOH Packet
	unsigned int SOH_analog_temp = 0;
	unsigned int SOH_digital_temp = 0;
	unsigned int SOH_module_temp = 0;
	unsigned int SOH_neutrons_0 = 0;
	unsigned int SOH_neutrons_1 = 0;
	unsigned int SOH_neutrons_2 = 0;
	unsigned int SOH_neutrons_3 = 0;
	unsigned int SOH_time = 0;
	unsigned char SOH_mode_byte = 0;	//see telemetry dictionary >v2.0
	unsigned int SOH_ID_number = 0;
	unsigned int SOH_Run_number = 0;
	//DIR Packets
	unsigned int dir_time = 0;
	//Data Product Packets
	int tx_id_number = 0;
	int tx_run_number = 0;
	//SUCCESS/FAIULRE Packet
	char CMD_command_text[100] = {};
	char MNS_command[100] = {};
	char MNS_filename[100] = {};
	char user_filename[100] = "";
	char user_filename_parsed[100] = "";
	char user_packets_array[120] = "";
	char user_data_bytes_array[120] = "";
	char user_binary_filename[120] = "";

	//get the name of the file we want to process from the user
	printf("Before entering filename to be read, place it in the folder with this executable.\n");
	printf("Enter a binary filename (without file type extension) to convert: \n");		// Get the input file that should be converted
	scanf(" %s", user_filename);
	sscanf( user_filename, "%s\n", user_filename_parsed);
	printf("|%s| is the filename requested\n", user_filename_parsed);

	//build the various filenames we want:
	strcpy(user_packets_array, user_filename_parsed);	strcpy(user_data_bytes_array, user_filename_parsed);	strcpy(user_binary_filename, user_filename_parsed);
	strcat(user_packets_array, "_PACKETS.txt");
	strcat(user_data_bytes_array, "_DATA_BYTES.bin");
	strcat(user_binary_filename, ".bin");
	printf("|%s| is the file to be parsed.\n", user_binary_filename);

	//set up output files
	//change this file name to change where data bytes are written //modified every time we get a new data product APID
	FILE *dataByteFile = NULL;
	//change this filename to change where the packet info is written
	FILE *packetInfoFile;
	packetInfoFile = fopen(user_packets_array, "w");
	if(packetInfoFile == NULL)
	{
		printf("Unable to open file for packet info\n");
		return -1;
	}
	//Notes: APID is the hex number from the byte
	//		same thing with detector number, sequence count, and packet length.
	//		Sequence count and packet length need to be combined and maybe shifted, but that's minor.
	//		With the group flags, there is a code which i'll use to indicate what type of packet it is:
	//		1 = first packet
	//		2 = intermediate packet
	//		3 = last packet
	//		4 = unsegmented packet
	fprintf(packetInfoFile, "Packet #: \tDet. Num \tAPID \tGroup Flags \tSeq. Count \tPacket Length \t|Anlg \tDigi \tModu| \tnPMT0 \tnPMT1 \tnPMT2 \tnPMT3| \tTime \tMode \tID \tRun \tSimple CHK \tFletcher CHK \tCCSDS CHK\n");
	//read in binary packet capture from TeraTerm
	//this is the input file //change file name to packets from TT
	FILE *getPacketCapture;
	getPacketCapture = fopen(user_binary_filename, "rb");
	if(getPacketCapture == NULL)
	{
		printf("Unable to open data file\n");
		return -1;
	}

	//get file size and read in entire contents into char buffer
	fseek(getPacketCapture, 0, SEEK_END);
	m_file_size = ftell(getPacketCapture);
	fseek(getPacketCapture, 0, SEEK_SET);
	printf("file size: %ld bytes\n\n", m_file_size);
	//allocate buffer of the correct size
	unsigned char *m_file_buff;
	m_file_buff = (unsigned char *)calloc(m_file_size, sizeof(unsigned char));
	if(m_file_buff == NULL)
	{
		printf("Bad allocation\n");
		return -2;
	}
	//read in the bytes from the file
	m_bytes_read = fread(m_file_buff, sizeof(unsigned char), m_file_size, getPacketCapture);
	if(m_bytes_read < m_file_size)
	{
		printf("Less than whole file read in\n");
		return -3;
	}
	//find sync marker and read next 6 bytes after that (the rest of the CCSDS header)
	//scan the char buffer we have read in
	while(m_iter < (m_file_size - 1))
	{
		//how much of the output file is processed?
		if(((float)m_iter / (float)m_file_size) * 100 >= 10 * (float)m_file_size_iter)
		{
			printf("%d %% of the file processed\n", 10 * m_file_size_iter);
			m_file_size_iter++;
		}
		//look for sync marker
		if(m_file_buff[m_iter] == 0x35 && m_file_buff[m_iter+1] == 0x2E && m_file_buff[m_iter+2] == 0xF8 && m_file_buff[m_iter+3] == 0x53)
		{
			if(m_file_buff[m_iter+5] != 0x00
					&& m_file_buff[m_iter+5] != 0x11
					&& m_file_buff[m_iter+5] != 0x22
					&& m_file_buff[m_iter+5] != 0x33
					&& m_file_buff[m_iter+5] != 0x44
					&& m_file_buff[m_iter+5] != 0x55
					&& m_file_buff[m_iter+5] != 0x66
					&& m_file_buff[m_iter+5] != 0x77
					&& m_file_buff[m_iter+5] != 0x88
				)
			{
				//then skip this sync marker because it's a fake
				m_iter++;
			}
			else
			{
				//found one, read the next 6 bytes (the rest of the header)
				m_packet_number++;
				m_detector_number = m_file_buff[m_iter + 4];
				m_packet_apid = m_file_buff[m_iter + 5];
				m_group_flags = (m_file_buff[m_iter+6] & 0xC0) >> 6;
				m_sequence_count = (m_file_buff[m_iter+6] &0x3F) << 8;
				m_sequence_count += m_file_buff[m_iter+7];
				m_packet_length = (m_file_buff[m_iter+8] << 8) + m_file_buff[m_iter+9];
				switch(m_group_flags)
				{
				case 0:
					m_group_flags = 0;	//intermediate packet
					break;
				case 1:
					m_group_flags = 1;	//first packet
					break;
				case 2:
					m_group_flags = 2;	//last packet
					break;
				case 3:
					m_group_flags = 3;	//unsegmented packet
					break;
				default:
					printf("Bad group flags, packet %d\n", m_packet_number);
					m_group_flags = 0;
					break;
				}
				fprintf(packetInfoFile, "Packet %d: \t%X \t%X \t%d \t%d \t%d \t", m_packet_number, m_detector_number, m_packet_apid, m_group_flags, m_sequence_count, m_packet_length);

				//check if the packet type is a data product or DIR packet
				if(m_packet_apid == 0x33 || m_packet_apid == 0x55 || m_packet_apid == 0x66 || m_packet_apid == 0x77 || m_packet_apid == 0x88)
				{
					if(m_group_flags == 1 || m_group_flags == 3)
					{
						//new data product file name if the packet is the first packet or unsegmented
						//this packet is the start of new data
						//get the id, run numbers
						if(m_packet_apid == 0x33)
						{
							dir_time = (m_file_buff[m_iter + 13] << 24) + (m_file_buff[m_iter + 14] << 16) + (m_file_buff[m_iter + 15] << 8) + m_file_buff[m_iter + 16];
							snprintf(user_data_bytes_array, 120, "dir_%u.bin", dir_time);
						}
						else
						{
							tx_id_number = m_file_buff[m_iter + 37];
							tx_run_number = m_file_buff[m_iter + 39];
							switch(m_packet_apid)
							{
							case 0x55:
								//CPS data product
								snprintf(user_data_bytes_array, 120, "cps_%d_%d.bin", tx_id_number, tx_run_number);
								break;
							case 0x66:
								//WF data product
								snprintf(user_data_bytes_array, 120, "wf_%d_%d.bin", tx_id_number, tx_run_number);
								break;
							case 0x77:
								//EVT data product
								snprintf(user_data_bytes_array, 120, "evt_%d_%d.bin", tx_id_number, tx_run_number);
								break;
							case 0x88:
								//2DH data product
								snprintf(user_data_bytes_array, 120, "2dh_%d_%d_%d.bin", m_file_buff[m_iter + 2033], tx_id_number, tx_run_number);
								break;
							default:
								//something not from our list
								break;
							}
						}

						//make sure the previous one is closed
//						fclose(dataByteFile);
						//open the new dataBytesFile with the name created above
						dataByteFile = fopen(user_data_bytes_array, "wb");
						if(dataByteFile == NULL)
						{
							printf("Unable to open file for data bytes\n");
							return -1;
						}
					}
				}

				if(m_packet_apid == 0x22)
				{
					//read the SOH packet in and print it's contents next to the
					//byte 11 is where packet information starts
					SOH_analog_temp =	(m_file_buff[m_iter + 11] << 24) + (m_file_buff[m_iter + 12] << 16) + (m_file_buff[m_iter + 13] << 8) + m_file_buff[m_iter + 14];
					SOH_digital_temp =	(m_file_buff[m_iter + 16] << 24) + (m_file_buff[m_iter + 17] << 16) + (m_file_buff[m_iter + 18] << 8) + m_file_buff[m_iter + 19];
					SOH_module_temp =	(m_file_buff[m_iter + 21] << 24) + (m_file_buff[m_iter + 22] << 16) + (m_file_buff[m_iter + 23] << 8) + m_file_buff[m_iter + 24];
					SOH_neutrons_0 =	(m_file_buff[m_iter + 26] << 24) + (m_file_buff[m_iter + 27] << 16) + (m_file_buff[m_iter + 28] << 8) + m_file_buff[m_iter + 29];
					SOH_neutrons_1 =	(m_file_buff[m_iter + 31] << 24) + (m_file_buff[m_iter + 32] << 16) + (m_file_buff[m_iter + 33] << 8) + m_file_buff[m_iter + 34];
					SOH_neutrons_2 =	(m_file_buff[m_iter + 36] << 24) + (m_file_buff[m_iter + 37] << 16) + (m_file_buff[m_iter + 38] << 8) + m_file_buff[m_iter + 39];
					SOH_neutrons_3 =	(m_file_buff[m_iter + 41] << 24) + (m_file_buff[m_iter + 42] << 16) + (m_file_buff[m_iter + 43] << 8) + m_file_buff[m_iter + 44];
					SOH_time =			(m_file_buff[m_iter + 46] << 24) + (m_file_buff[m_iter + 47] << 16) + (m_file_buff[m_iter + 48] << 8) + m_file_buff[m_iter + 49];
					SOH_mode_byte =		 m_file_buff[m_iter + 51];
					SOH_ID_number = 	(m_file_buff[m_iter + 53] << 24) + (m_file_buff[m_iter + 54] << 16) + (m_file_buff[m_iter + 55] << 8) + m_file_buff[m_iter + 56];
					SOH_Run_number = 	(m_file_buff[m_iter + 58] << 24) + (m_file_buff[m_iter + 59] << 16) + (m_file_buff[m_iter + 60] << 8) + m_file_buff[m_iter + 61];
					m_simple_checksum = m_file_buff[m_iter + 63];
					m_Fletcher_checksum = m_file_buff[m_iter + 64];
					m_ccsds_checksum = (m_file_buff[m_iter + 65] << 8) + m_file_buff[m_iter + 66];
					//could calculate the checksums here if we wanted to compare and ensure that we're getting the correct values

					fprintf(packetInfoFile, "|%d \t%d \t%d| \t%d \t%d \t%d \t%d| \t%d \t%d| \t%d \t%d| \t%X \t%X \t%X\n",
							SOH_analog_temp,
							SOH_digital_temp,
							SOH_module_temp,
							SOH_neutrons_0,
							SOH_neutrons_1,
							SOH_neutrons_2,
							SOH_neutrons_3,
							SOH_time,
							SOH_mode_byte,
							SOH_ID_number,
							SOH_Run_number,
							m_simple_checksum,
							m_Fletcher_checksum,
							m_ccsds_checksum);

					m_iter += (m_packet_length + CCSDS_HEADER_PRIM);
					//we're now at the next packet, we have passed over the checksums
				}
				else if(m_packet_apid == 0x33)
				{
					//this is a DIR packet
					m_header_bytes = 17 + CCSDS_HEADER_FULL;
					m_total_packet_length = 2006;

					if(m_group_flags == 1 || m_group_flags == 3)
					{
						//write the header bytes into the file for first, unsegmented packets
						m_bytes_written = fwrite(&(m_file_buff[m_iter + CCSDS_HEADER_FULL]), sizeof(unsigned char), 17, dataByteFile);
						if(m_bytes_written < 17)
							printf("Error writing to data byte file, only wrote %d bytes for packet %d\n", m_bytes_written, m_packet_number);
					}

					m_bytes_written = fwrite(&(m_file_buff[m_iter + m_header_bytes]), sizeof(unsigned char), m_total_packet_length, dataByteFile);
					if(m_bytes_written < m_total_packet_length)
						printf("Error writing to data byte file, only wrote %d bytes for packet %d\n", m_bytes_written, m_packet_number);
					else
						fprintf(packetInfoFile, "%d\t", m_total_packet_length );

					//if the packet is unsegmented or a last packet, close the file
					if(m_group_flags == 2 || m_group_flags == 3)
						fclose(dataByteFile);

					//validate the checksums
					m_checksum_match = verifyCRC(&(m_file_buff[m_iter]), m_packet_length);
					fprintf(packetInfoFile, "%X \n", m_checksum_match );
					//now skip over these bytes and begin looking for the next sync marker
					m_iter += (m_packet_length + CCSDS_HEADER_PRIM);

				}
				else if(m_packet_apid == 0x44)
				{
					//read the temperature packet in and print it's contents next to the
					//byte 11 is where packet information starts
					SOH_analog_temp =	(m_file_buff[m_iter + 11] << 24) + (m_file_buff[m_iter + 12] << 16) + (m_file_buff[m_iter + 13] << 8) + m_file_buff[m_iter + 14];
					SOH_digital_temp =	(m_file_buff[m_iter + 16] << 24) + (m_file_buff[m_iter + 17] << 16) + (m_file_buff[m_iter + 18] << 8) + m_file_buff[m_iter + 19];
					SOH_module_temp =	(m_file_buff[m_iter + 21] << 24) + (m_file_buff[m_iter + 22] << 16) + (m_file_buff[m_iter + 23] << 8) + m_file_buff[m_iter + 24];
					m_simple_checksum = m_file_buff[m_iter + 26];
					m_Fletcher_checksum = m_file_buff[m_iter + 27];
					m_ccsds_checksum = (m_file_buff[m_iter + 28] << 8) + m_file_buff[m_iter + 29];
					//could calculate the checksums here if we wanted to compare and ensure that we're getting the correct values

					fprintf(packetInfoFile, "|%d \t%d \t%d| \t%X \t%X \t%X\n",
							SOH_analog_temp,
							SOH_digital_temp,
							SOH_module_temp,
							m_simple_checksum,
							m_Fletcher_checksum,
							m_ccsds_checksum);

					m_iter += (m_packet_length + CCSDS_HEADER_PRIM);
					//we're now at the next packet, we have passed over the checksums
				}
				else if(m_packet_apid == 0x55 || m_packet_apid == 0x66 || m_packet_apid == 0x77 || m_packet_apid == 0x88)
				{
					//this writes the entirety of the data bytes, but does remove the RMD data headers which are not true data
					//need to output the Mini-NS Data Header somewhere
					if(m_packet_apid == 0x55) //CPS
						m_header_bytes = 39 + CCSDS_HEADER_PRIM;
					if(m_packet_apid == 0x66) //WAV
						m_header_bytes = 39 + CCSDS_HEADER_PRIM;
					if(m_packet_apid == 0x77) //EVT
						m_header_bytes = 39 + CCSDS_HEADER_PRIM;
					if(m_packet_apid == 0x88) //2DH
						m_header_bytes = 39 + CCSDS_HEADER_PRIM;
					//write these bytes into the data bytes output file minus the checksums
					//there are packet length bytes + 1 total in the data bytes,
					// minus 4 for the checksums
					//also, move past the header bytes then read that many fewer bytes from the packet
					m_total_packet_length = m_packet_length + 1 + CCSDS_HEADER_PRIM - 4 - m_header_bytes;	//the packet length already took into account the CCSDS header bytes
					if(m_packet_apid == 0x88)
						m_total_packet_length -= 1;	//subtract the PMT ID byte from the 2DH file
					//but we don't want to read through the checksums
					m_bytes_written = fwrite(&(m_file_buff[m_iter + m_header_bytes]), sizeof(unsigned char), m_total_packet_length, dataByteFile);
					if(m_bytes_written < m_total_packet_length)
						printf("Error writing to data byte file, only wrote %d bytes for packet %d\n", m_bytes_written, m_packet_number);
					else
						fprintf(packetInfoFile, "%d\t", m_total_packet_length );

					//if the packet is unsegmented or a last packet, close the file
					if(m_group_flags == 2 || m_group_flags == 3)
						fclose(dataByteFile);

					//validate the checksums
					m_checksum_match = verifyCRC(&(m_file_buff[m_iter]), m_packet_length);
					fprintf(packetInfoFile, "%X \n", m_checksum_match );
					//now skip over these bytes and begin looking for the next sync marker
					m_iter += (m_packet_length + CCSDS_HEADER_PRIM);
				}
				else if(m_packet_apid == 0x00 || m_packet_apid == 0x11)
				{
					//this is the SUCCESS/FAILURE packet section
					//print the text from the packet
					for(i = 0; i < m_packet_length - 4; i++)
					{
						CMD_command_text[i] = m_file_buff[m_iter + 11 + i];
					}

					sscanf(CMD_command_text, "%s\n%s", MNS_command, MNS_filename);

					fprintf(packetInfoFile, "%s - %s\n", MNS_command, MNS_filename);
					memset(CMD_command_text, '\0', sizeof(char) * 100);
					m_iter += (m_packet_length + CCSDS_HEADER_PRIM);
				}
				else
				{
					printf("Got a weird APID, check packet %d\n", m_packet_number);
					m_iter++;	//don't go far
				}
			}
		}
		else
		{
			//if we don't have a sync marker, we don't care about the bytes
			//printf("skipped byte %d\n", m_iter);
			m_iter++;
		}
	}
	printf("%d%% of the file processed\n", 100);
	//read in the packet length number of bytes and write to a data bytes file

	//that's it, clean up
	fclose(packetInfoFile);
	fclose(dataByteFile);
	fclose(getPacketCapture);
	free(m_file_buff);

	char m_string[100];
	printf("\nPress 'q' and enter to quit...\n");
	while(scanf("%s",m_string) == 1)
	{
		if(m_string[0] == 'q')
			break;
	}
	return 0;
}



