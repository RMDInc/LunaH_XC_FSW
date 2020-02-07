// V5 Raw Data File Reader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <array>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main()
{
	//want to simply read in the file, invert the bytes to make ints, then save to a text file
	//so binary -> read as u-ints -> print to file
	long long i{};
	long long file_length{};
	int m_file_size_iter{};
	int m_printed{};
	string input = "raw_data";
	string infile{};
	string outfile{};
	string outfile2{};
	ifstream ifs;
	ofstream ofs;

	//ask user for the filename that we are opening to process
	cout << "Before entering file to read, place it in the folder with this executable.\n";
	cout << "Binary filename (without file type extension) to convert: \n";
	cout << input << endl;
	//cout << "The filename is case-sensitive." << endl;
	//getline(cin, input);
	infile = input + ".bin";
	outfile = input + "_translated.txt";

	//open the stream
	ifs.open(infile, ios_base::in | ios_base::binary);
	istreambuf_iterator<char> start(ifs.rdbuf()), end_of_stream;

	cout << "Read in the file..." << endl;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	vector<unsigned char> file_buff(start, end_of_stream);	//read in the whole file
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	cout << "...done" << endl;
	auto duration = duration_cast<seconds>(t2 - t1).count();
	cout << "Reading the file took " << duration << " seconds." << endl;

	//open the output file
	ofs.open(outfile);
	cout << "Write the events to a file..." << endl;

	//set up for processing
	unsigned int test_val1 = 0;
	unsigned int test_val2 = 0;
	unsigned int pmt_hit_id = 0;
	file_length = file_buff.size();
	while (i < file_length - 35)
	{
		//find matching event IDs
		test_val1 = (file_buff[i + 3] << 24) | (file_buff[i + 2] << 16) | (file_buff[i + 1] << 8) | (file_buff[i + 0]);
		if (test_val1 == 111111 || test_val1 == 2147594759 || test_val1 ==  1073852935 )
		{
			test_val2 = (file_buff[i + 3 + 4] << 24) | (file_buff[i + 2 + 4] << 16) | (file_buff[i + 1 + 4] << 8) | (file_buff[i + 0 + 4]);
			if (test_val2 == 111111)
			{
				i++;
			}
			else
			{
				for (size_t j = 0; j < 8; j++)
				{
					if (j == 3)
					{
						test_val2 = ((file_buff[i + 3 + (4 * j)] << 24) | (file_buff[i + 2 + (4 * j)] << 16) | (file_buff[i + 1 + (4 * j)] << 8) | (file_buff[i + 0 + (4 * j)]));
						pmt_hit_id = test_val2 & 0x0F;
						test_val2 = (test_val2 >> 4) & 0x0FFFFFFF;
						ofs << test_val2 << '\t' << pmt_hit_id;
					}
					else
						ofs << ((file_buff[i + 3 + (4 * j)] << 24) | (file_buff[i + 2 + (4 * j)] << 16) | (file_buff[i + 1 + (4 * j)] << 8) | (file_buff[i + 0 + (4 * j)]));

					if (j < 7)
						ofs << '\t';
					else
					{
						ofs << '\n';
						m_printed = 1;
					}
				}
			}
			if (m_printed == 1)
			{
				i++;
				m_printed = 0;
			}
		}
		else
			i++;

		if (((float)i / (float)file_length) * 100 >= 10 * (float)m_file_size_iter)
		{
			cout << 10 * m_file_size_iter << " % of the file processed" << endl;
			m_file_size_iter++;
		}
	}

	ifs.close();
	ofs.close();

	cout << "Done sorting." << endl;

	//system("pause");
	return 0;
}

