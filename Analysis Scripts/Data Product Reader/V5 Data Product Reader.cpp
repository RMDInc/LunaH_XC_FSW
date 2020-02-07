// V4 Data Product Reader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <array>
#include <vector>

#define X_BINS			512
#define Y_BINS			64
#define DIR_PACKET_SIZE	2027
#define CHECKSUM_SIZE	4

using namespace std;

int process_file(string input = "")
{
	//variables
	char input_file_type[4]{};	int size_of_input_file_type = sizeof(input_file_type);
	char m_char_buff[10]{};		int sizeof_m_char_buff = sizeof(m_char_buff);
	char dir_name_buffer[100]{};
	char dir_scanned_name[100]{};
	char dir_scanned_fsize[10]{};
	int j = 0;
	int k{};
	int iter = 0;
	int dir_chars{};
	int dir_bytes_scanned{};
	int events_read = 0;
	int skipped_iter = 0;
	int skipped_event_count = 0;
	int skipped_pmt_event_count = 0;
	int skipped_non_zero_pmt_id_event{};
	int event_diff = 0;
	int scan_ret = 0;
	int input_set = 0;
	int input_type = 0;
	int m_wf_type{};
	int m_2dh_type{};
	int m_id_number{};
	int m_run_number{};
	int m_pmt_id{};
	int m_total_tagged_events{};
	int status{};
	unsigned __int64 i = 0;	//this avoids the warning that we could overflow i
	unsigned int pmt_id = 0;
	unsigned int dir_sd_card_num{};
	unsigned int dir_real_time{};
	unsigned int dir_total_folders{};
	unsigned int dir_total_files{};
	unsigned int dir_file_size{};
	unsigned int event_num{};
	unsigned int aggregated_events{};
	unsigned int event_holder{};
	unsigned int energy_bin = 0;
	unsigned int psd_bin = 0;
	unsigned int m_tagging_bit{};
	unsigned int fpga_time = 0;
	unsigned int cps_neutrons_with_PSD = 0;
	unsigned int cps_neutrons_wide_cut = 0;
	unsigned int cps_non_neutron_events = 0;
	unsigned int cps_events_over_threshold = 0;
	unsigned int cps_module_temp = 0;
	unsigned int m_wf_sample_val{};
	array<array<unsigned short, Y_BINS>, X_BINS> twoDH_total{};
	array<array<unsigned short, Y_BINS>, X_BINS> twoDH_PMT{};
	string infile = "";
	string outfile = "";
	string outfile_evt_2dh = "";
	string outfile_pmt_2dh = "";
	ifstream ifs;
	ofstream ofs_data;
	ofstream ofs_evt_2dh;
	ofstream ofs_pmt_2dh;

	//ask user for the filename that we are opening to process
	cout << "Before entering file to read, place it in the folder with this executable.\n";
	cout << "Binary filename (without file type extension) to convert: \n";		// Get the input file that should be converted
	cout << input << endl;
	//getline(cin, input);
	infile = input + ".bin";
	outfile = input + "_translated.txt";
	outfile_evt_2dh = input + "_EVT_2DH.txt";
	outfile_pmt_2dh = input + "_PMT_2DH.txt";

	//scan the input for what kind of file it is (CPS, EVT, 2DH) and echo the ID, run, Set numbers
	scan_ret = sscanf_s(input.c_str(), " %[^_]", input_file_type, size_of_input_file_type);
	//Types:
	//	dir = 33
	//	cps = 55
	//  wav = 66
	//	evt = 77
	//	2d1 = 88
	//	2d2 = 88
	//	2d3 = 88
	//	2d4 = 88

	//strcmp() is case sensitive, can change to strcasecmp() or stricmp() if there a problem in the future
	if (strcmp(input_file_type, "dir") == 0)
	{
		input_type = 33;
		scan_ret = sscanf_s(input.c_str(), " %*[^_]_%d", &dir_real_time);
		if (scan_ret != 1)
			status = 1;
	}
	else if (strcmp(input_file_type, "cps") == 0)
	{
		input_type = 55;
		scan_ret = sscanf_s(input.c_str(), " %*[^_]_%d_%d", &m_id_number, &m_run_number);
		if (scan_ret != 2)
		{
			scan_ret = sscanf_s(input.c_str(), " %*[^_]_S%04d", &input_set);
			if (scan_ret != 1)
			{
				scan_ret = sscanf_s(input.c_str(), " %[^.].bin", m_char_buff, sizeof_m_char_buff);
				if (scan_ret != 1)
					status = 1;
			}
		}
	}
	else if (strcmp(input.c_str(), "wf01") == 0)
	{
		input_type = 66;
		m_wf_type = 1;	//SD card file
	}
	else if (strcmp(input_file_type, "wf") == 0)
	{
		input_type = 66;
		scan_ret = sscanf_s(input.c_str(), " %*[^_]_%d_%d", &m_id_number, &m_run_number);
		if (scan_ret != 2)
			status = 1;
		else
			m_wf_type = 2;	//transferred WF file
	}
	else if (strcmp(input_file_type, "evt") == 0)
	{
		input_type = 77;
		scan_ret = sscanf_s(input.c_str(), " %*[^_]_%d_%d", &m_id_number, &m_run_number);
		if (scan_ret != 2)
		{
			scan_ret = sscanf_s(input.c_str(), " %*[^_]_S%04d", &input_set);
			if (scan_ret != 1)
				status = 1;
		}
	}
	else if (strcmp(input_file_type, "2dh") == 0)
	{
		input_type = 88;
		m_2dh_type = 1;		//the parsed file 2dh type
		scan_ret = sscanf_s(input.c_str(), " %*[^_]_%d_%d_%d", &m_pmt_id, &m_id_number, &m_run_number);
		if (scan_ret != 3)
			status = 1;
	}
	else if (strcmp(input_file_type, "2d0") == 0 || strcmp(input_file_type, "2d1") == 0 || strcmp(input_file_type, "2d2") == 0 || strcmp(input_file_type, "2d3") == 0)
	{
		input_type = 88;
		//when doing a scan like this, case does matter
/*		scan_ret = sscanf_s(input.c_str(), " %[^_]_S%d", m_char_buff, sizeof_m_char_buff, &input_set);
		if (scan_ret != 2)
			status = 1;
		else
		{ */
		scan_ret = sscanf_s(input.c_str(), " %[^.].bin", m_char_buff, sizeof_m_char_buff);
		if (scan_ret != 1)
			status = 1;
		else
		{
			m_2dh_type = 2;	//the SD card file 2dh type
			if (strcmp(m_char_buff, "2d0") == 0)
				m_pmt_id = 1;
			else if (strcmp(m_char_buff, "2d1") == 0)
				m_pmt_id = 2;
			else if (strcmp(m_char_buff, "2d2") == 0)
				m_pmt_id = 3;
			else if (strcmp(m_char_buff, "2d3") == 0)
				m_pmt_id = 4;
			else
				status = 1;
		}
	}
	else
	{
		cout << "Error getting file type. Please use a file which was parsed with the Output Reader." << endl;
		system("pause");
		return 100;
	}

	if (status == 1)
	{
		cout << "Error reading the file name. Did not recognize the ID/Run/Set number or filename." << endl;
		system("pause");
		return status;
	}

	cout << "New file will be named:" + outfile + "\n";

	ifs.open(infile, ios_base::binary);
	istreambuf_iterator<char> start(ifs.rdbuf()), end_of_stream;
	cout << "Read in the file..." << endl;
	vector<unsigned char> file_buff(start, end_of_stream);	//read in the whole file
	cout << "...done. File size = " << file_buff.size() << endl;

	if (file_buff.size() <= 0)
	{
		cout << "The file you are trying to process does not exist." << endl;
		system("pause");
		return 100;
	}

	//open the file and write the header in depending on the type of file
	ofs_data.open(outfile);
	ofs_data << "File translated: " << infile << endl;
	if (input_type == 33)	//dir
	{
		ofs_data << "File Name\tFile Size (bytes)" << endl;
	}
	else if (input_type == 55)	//cps
	{
		ofs_data << "Neutrons Cut 1\tNeutrons Cut 2\tNon-Neutron Events\tHigh Energy Events\tTime\tModule Temp" << endl;
	}
	else if (input_type == 66)	//wav
	{
		ofs_data << "Sample value" << endl;
	}
	else if (input_type == 77)	//evt
	{
		ofs_data << "PMT ID\tTotal Events\tAggregated Events\tEnergy Bin\tPSD Bin\tTagging Bit\tFPGA Time" << endl;

		//this file lets us create a 2DH-like histogram by processing the EVT data
		ofs_evt_2dh.open(outfile_evt_2dh);
		ofs_evt_2dh << "These are the 2DH values from the data file" << infile << endl;
		//		ofs_evt_2dh << "Values start with array[0][0] then array[0][1] etc, ending with array[512][64]" << endl;

				//this file is the same as the EVT_2DH, but it only registers an event if the PMT ID was > 0
		ofs_pmt_2dh.open(outfile_pmt_2dh);
		ofs_pmt_2dh << "These are the 2DH values from the 2DH data file" << infile << endl;
		//		ofs_pmt_2dh << "Values start with array[0][0] then array[0][1] etc, ending with array[512][64]" << endl;
	}
	else if (input_type == 88)
	{
		//		ofs_data << "These are the 2DH values from the data file" << infile << endl;
		//		ofs_data << "Values start with array[0][0] then array[0][1] etc, ending with array[512][64]" << endl;
	}
	else
	{
		cout << "Error unknown file type." << endl;
		system("pause");
		return 101;
	}

	cout << "Writing events to the file..." << endl;
	i = 0;
	if (input_type == 33)
	{
		//find and read in the header bytes
		while (i < file_buff.size() - 16)
		{
			//read through the header //17 bytes
			if (file_buff[i + 1] == 0x0A
				&& file_buff[i + 6] == 0x0A
				&& file_buff[i + 11] == 0x0A
				&& file_buff[i + 16] == 0x0A)
			{
				dir_sd_card_num = file_buff[i];
				dir_real_time = (file_buff[i + 2] << 24) + (file_buff[i + 3] << 16) + (file_buff[i + 4] << 8) + (file_buff[i + 5]);
				dir_total_folders = (file_buff[i + 7] << 24) + (file_buff[i + 8] << 16) + (file_buff[i + 9] << 8) + (file_buff[i + 10]);
				dir_total_files = (file_buff[i + 12] << 24) + (file_buff[i + 13] << 16) + (file_buff[i + 14] << 8) + (file_buff[i + 15]);
				i += 17; k += 17;
				break;
			}
			else
			{
				i++; k++;
			}
		}
		//loop over the packet contents
		while (i < file_buff.size())
		{
			while (file_buff[i] != 0x0A)	//loop until we find a newline
			{
				i++; k++;
				dir_chars++;
				if (k == DIR_PACKET_SIZE - CHECKSUM_SIZE)
				{
					if (file_buff[i - 1] == 0x03 && file_buff[i - 2] == 0x03 && file_buff[i - 3] == 0x03 && file_buff[i - 4] == 0x03 && file_buff[i - 5] == 0x03)
					{
						//if the previous five values were all padding bytes, then these are all padding bytes, skip them and reset iterator
						k = 17;
						dir_chars = 0;
						if (i >= file_buff.size())
							break;
					}
				}
			}
			if (i >= file_buff.size())
				break;
			memcpy(dir_name_buffer, (const char*)&file_buff[i - dir_chars], dir_chars);
			//strncpy_s(dir_name_buffer, (const char*)&file_buff[i - dir_chars], dir_chars);

			scan_ret = sscanf_s(dir_name_buffer, " %s%n", dir_scanned_name, 100, &dir_bytes_scanned);
			if (scan_ret == 1)
			{
				//we found at least something, what is the whitespace that stopped the read?
				//if it is a newline	//we found just a folder, write that don't look for file size
				//if it is a tab		//we found a file, read the file size
				if (file_buff[i - dir_chars + dir_bytes_scanned] == 0x09)
				{
					//read the file size //we found a file name and size
					dir_file_size = (file_buff[i - 4] << 24) + (file_buff[i - 3] << 16) + (file_buff[i - 2] << 8) + file_buff[i - 1];
					ofs_data << dir_scanned_name << '\t' << dir_file_size << endl;
					i++; k++;
					dir_chars = 0;
				}
				else
				{
					ofs_data << dir_scanned_name << endl;
					i++; k++;
					dir_chars = 0;
				}
				memset(dir_scanned_name, '\0', sizeof(dir_scanned_name));
			}
			else
			{
				status = 1;
				break;
			}
		}
	}
	else if (input_type == 55)
	{
		while (i < (file_buff.size() - 14))
		{
			//handle reading CPS files here
			//check for filler bytes at the end of the file
			if (file_buff[i] == 0x55
				&& file_buff[i + 2] == 0x55
				&& file_buff[i + 3] == 0x55
				&& file_buff[i + 4] == 0x55
				&& file_buff[i + 5] == 0x55
				&& file_buff[i + 6] == 0x55
				&& file_buff[i + 7] == 0x55)
				break;
			if (file_buff[i] == 0x55 && file_buff[i + 14] == 0x55)	//CPS Data Product
			{
				cps_neutrons_with_PSD = (file_buff[i + 1] << 8) + file_buff[i + 2];
				cps_neutrons_wide_cut = (file_buff[i + 3] << 8) + file_buff[i + 4];
				cps_non_neutron_events = (file_buff[i + 5] << 8) + file_buff[i + 6];
				cps_events_over_threshold = (file_buff[i + 7] << 8) + file_buff[i + 8];
				fpga_time = ((file_buff[i + 9] & 0x03) << 24) + (file_buff[i + 10] << 16) + (file_buff[i + 11] << 8) + (file_buff[i + 12]);
				cps_module_temp = file_buff[i + 13];

				ofs_data << cps_neutrons_with_PSD << "\t"
					<< cps_neutrons_wide_cut << "\t"
					<< cps_non_neutron_events << "\t"
					<< cps_events_over_threshold << "\t"
					<< fpga_time << "\t"
					<< cps_module_temp << endl;
				i += 14;
				events_read++;
			}
			else if (file_buff[i] == 0x55 && file_buff[i + 14] == 0xFF && file_buff[i + 15] == 0x45 && file_buff[i + 16] == 0x4E && file_buff[i + 17] == 0x44)	//Final CPS Data Product
			{
				cps_neutrons_with_PSD = (file_buff[i + 1] << 8) + file_buff[i + 2];
				cps_neutrons_wide_cut = (file_buff[i + 3] << 8) + file_buff[i + 4];
				cps_non_neutron_events = (file_buff[i + 5] << 8) + file_buff[i + 6];
				cps_events_over_threshold = (file_buff[i + 7] << 8) + file_buff[i + 8];
				fpga_time = ((file_buff[i + 9] & 0x03) << 24) + (file_buff[i + 10] << 16) + (file_buff[i + 11] << 8) + (file_buff[i + 12]);
				cps_module_temp = file_buff[i + 13];

				ofs_data << cps_neutrons_with_PSD << "\t"
					<< cps_neutrons_wide_cut << "\t"
					<< cps_non_neutron_events << "\t"
					<< cps_events_over_threshold << "\t"
					<< fpga_time << "\t"
					<< cps_module_temp << endl;
				i += 14;
				events_read++;
			}
			else
			{
				i++;
				skipped_iter++;
			}
			if (i % 100000 == 0)
				cout << "Read " << i << " values. Processed " << events_read << " events." << endl;
		}
	}
	else if (input_type == 66)
	{
		//we should make sure that the file is a multiple of 4 so that we are reading in the correct values
		// i'm not sure how to do that...

		//handle reading waveforms here
		//first kind is the SD card file with a header, move past the header and start reading
		//the second kind is the transferred kind, which has no header, just start reading
		if (m_wf_type == 1)
			i = 336;// i = 320;	//the config file changed 1/10/2020

		while (i < (file_buff.size() - 4))
		{
			m_wf_sample_val = (file_buff[i + 3] << 24) + (file_buff[i + 2] << 16) + (file_buff[i + 1] << 8) + (file_buff[i]);
			if (m_wf_sample_val == 1717986918)	//0x66666666
				break;

			ofs_data << m_wf_sample_val << endl;
			i += 4;
			events_read++;

			if (i % 100000 == 0)
				cout << "Read " << i << " values. Processed " << events_read << " events." << endl;
		}
	}
	else if (input_type == 77)
	{
		//handle reading EVT here
		while (i < (file_buff.size() - 8))
		{
			if (file_buff[i] == 0xFF && file_buff[i + 8] == 0xFF)	//Event Data Product
			{
				if (file_buff[i + 1] == 0x45 && file_buff[i + 2] == 0x4E && file_buff[i + 3] == 0x44)
				{
					i += 20;
					break;
				}
				pmt_id = file_buff[i + 1] >> 4;
				event_num = ((file_buff[i + 1] & 0x0F) << 8) + (file_buff[i + 2]);		//12 bits
				energy_bin = (file_buff[i + 3] << 1) + ((file_buff[i + 4] & 0x80) >> 7);	//9 bits
				psd_bin = ((file_buff[i + 4] & 0x7E) >> 1);									//6 bits
				m_tagging_bit = file_buff[i + 4] & 0x01;
				// fpga_time = 2^24*4ns per tick = 6.7108864E-2 sec per tick
				fpga_time = (file_buff[i + 5] << 24) + (file_buff[i + 6] << 16) + (file_buff[i + 7] << 8);	//24 bits

				//record the bin which was hit
				if (energy_bin < X_BINS && psd_bin < Y_BINS)
					twoDH_total[energy_bin][psd_bin]++;
				else
					skipped_event_count++;

				//if the PMT ID is not 0, create a second 2DH
				//will probably want to expand this to create the 2DH for each PMT
				if (pmt_id != 0)
				{
					switch (pmt_id)
					{
					case 1:
						//pmt 1 only
						break;
					case 2:
						//pmt 2 only
						break;
					case 4:
						//pmt 3 only
						break;
					case 8:
						//pmt 4 only
						break;
					default:
						//catch events which are multi-hits
						break;
					}

					if (energy_bin < X_BINS && psd_bin < Y_BINS)
						twoDH_PMT[energy_bin][psd_bin]++;
					else
						skipped_non_zero_pmt_id_event++;
				}
				else
					skipped_pmt_event_count++;

				if (m_tagging_bit == 1)
					m_total_tagged_events++;
				//count the event total
				//set the first event as 0, then compute the difference between current total and the holder
				//this allows us to count up even though the real counter rolls over
				event_diff = event_num - event_holder;
				if (event_diff < 0)
				{
					event_diff += 4096;
				}
				aggregated_events += event_diff;
				event_holder = event_num;

				ofs_data << pmt_id << "\t"
					<< event_num << "\t"
					<< aggregated_events << "\t"
					<< energy_bin << "\t"
					<< psd_bin << "\t"
					<< m_tagging_bit << "\t"
					<< fpga_time << endl;
				i += 8;
				events_read++;
			}
			else if (file_buff[i] == 0xDD && file_buff[i + 1] == 0xDD && file_buff[i + 2] == 0xDD && file_buff[i + 3] == 0xDD && file_buff[i + 4] == 0xDD)	//First Event Data Product
			{
				// fpga_time = 2^24*4ns per tick = 6.7108864E-2 sec per tick
				fpga_time = (file_buff[i + 5] << 24) + (file_buff[i + 6] << 16) + (file_buff[i + 7] << 8);

				ofs_data << 221 << "\t"
					<< 221 << "\t"
					<< 221 << "\t"
					<< 221 << "\t"
					<< 221 << "\t"
					<< fpga_time << endl;
				i += 8;
				events_read++;
			}
			else if (file_buff[i] == 0xEE && file_buff[i + 1] == 0xEE && file_buff[i + 2] == 0xEE && file_buff[i + 3] == 0xEE && file_buff[i + 4] == 0xEE)	//Pulser Event Data Product
			{
				// fpga_time = 2^24*4ns per tick = 6.7108864E-2 sec per tick
				fpga_time = (file_buff[i + 5] << 24) + (file_buff[i + 6] << 16) + (file_buff[i + 7] << 8);

				ofs_data << 238 << "\t"
					<< 238 << "\t"
					<< 238 << "\t"
					<< 238 << "\t"
					<< 238 << "\t"
					<< fpga_time << endl;
				i += 8;
				events_read++;
			}
			else
			{
				i++;
				skipped_iter++;
			}
			if (i % 100000 == 0)
				cout << "Read " << i << " values. Processed " << events_read << " events." << endl;
		}

		//make the specific extra 2DH files here
		cout << "Creating a test 2DH output file from all of the EVT data .";
		iter = Y_BINS - 1;
		while (iter > -1)
		{
			for (j = 0; j < X_BINS; j++)
			{
				ofs_evt_2dh << twoDH_total[j][iter] << "\t";
			}
			ofs_evt_2dh << endl;
			iter--;
		}
		cout << "...done." << endl;
		cout << "Creating a second test 2DH output file from only the EVT data with non-zero PMT IDs.";
		iter = Y_BINS - 1;
		while (iter > -1)
		{
			for (j = 0; j < X_BINS; j++)
			{
				ofs_pmt_2dh << twoDH_PMT[j][iter] << "\t";
			}
			ofs_pmt_2dh << endl;
			iter--;
		}
		cout << "...done." << endl;
	}
	else if (input_type == 88)
	{
		//2DH type 1 = TX file
		//2DH type 2 = SD card file
		if (m_2dh_type == 2)
		{
			i = 336;	//skip past the number of bytes in the 2dh header //328 bytes in type 2 2dh header
			cout << endl << "Processing type 2 (SD card) 2DH file" << endl;
		}
		else
		{
			i = 0;
			cout << endl << "Processing type 1 (Transfer) 2DH file" << endl;
		}
		//loop over all values in the 2dh file and assign them to an array index [x][y]
		//this assumes that the values in the 2dh data file go from array[0][0] -> array[63][511]
		//try doing array[0][0] -> array[0][29], array[1][0] -> array[1][63], ...
		j = 0;
		while (j < X_BINS)
		{
			for (iter = 0; iter < Y_BINS; iter++)
			{
				twoDH_total[j][iter] = ((file_buff[i + 1] << 8) + file_buff[i]);
				i += 2;
			}
			j++;
		}

		iter = Y_BINS - 1;
		while (iter > -1)
		{
			for (j = 0; j < X_BINS; j++)
			{
				ofs_data << twoDH_total[j][iter] << "\t";
			}
			ofs_data << endl;
			iter--;
		}

		if (m_2dh_type == 2)
		{
			cout << "The out-of-range values were: " << endl
				<< "Left: " << ((file_buff[i + 1] << 8) + file_buff[i]) << endl
				<< "Right: " << ((file_buff[i + 3] << 8) + file_buff[i + 2]) << endl
				<< "Below: " << ((file_buff[i + 5] << 8) + file_buff[i + 4]) << endl
				<< "Above: " << ((file_buff[i + 7] << 8) + file_buff[i + 6]) << endl
				<< "Valid multi-hit: " << ((file_buff[i + 9] << 8) + file_buff[i + 8]) << endl;
		}
	}

	if (status == 1)
	{
		cout << "Error parsing from the file. Problem reading a file name/size or folder name." << endl;
		system("pause");
		return status;
	}

	cout << "...done." << endl;

	if (input_type == 55 || input_type == 66 || input_type == 77)
	{
		cout << events_read << " is the number of events processed." << endl;
		if (input_type == 55 || input_type == 77)
		{
			cout << skipped_iter << " is the number of skipped iterators." << endl;
			if (input_type == 77)
			{
				cout << skipped_event_count << " is the number of EVT events which did not have bin numbers inside 512 x 64." << endl;
				cout << skipped_non_zero_pmt_id_event << " is the number of EVT events which had a non-zero PMT ID, but were outside 512 x 64." << endl;
				cout << skipped_pmt_event_count << " is the number of EVT events which had a PMT ID of zero." << endl;
				cout << m_total_tagged_events << " is the number of tagged events." << endl;
			}
		}
	}

	ifs.close();
	ofs_data.close();
	ofs_evt_2dh.close();
	ofs_pmt_2dh.close();

	//system("pause");
	return 0;
}




//added an override in the Properties > Configuration > C/C++ > Command Line
// /analyze:stacksize147456 is the override
// which will try and give more stack space? I am not sure this can be done (as in this command will be ignored).
//The warning does not go away with the addition of the override.
int main()
{

	process_file("2d0");
	process_file("2d1");
	process_file("2d2");
	process_file("2d3");
	process_file("cps");
	process_file("evt_S0000");
	return 0;
}

