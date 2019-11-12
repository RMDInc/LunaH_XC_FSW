The scripts in this folder can be useful for processing binary output from the Mini-NS. 

When using these programs, note that syntax is important, otherwise the programs will not find the files you want to process.

 - Packet Reader
	- this program will process the packetized output from the Mini-NS. It will handle all output packet types and scrape out any data products into separate files. Each data product will get its own file. Data products from transfers should be processed using this program. 
	- files are accepted by this program in binary format

 - Data Product Reader
	- this program takes in data product files from either the SD card or from the Packet Reader program. When processing a file with this program, the name of the file to be read, including capitalization, must be exactly the same. 
	- this program takes in binary files
	
 - Raw File Processor
	- this program does not have a use with version 6.4