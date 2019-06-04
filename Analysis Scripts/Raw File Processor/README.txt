This script will run over the 2DH, CPS, or EVT data product raw files taken from the SD card and produce a set of text files which yield different information about the file.

Place the files to be processed into a folder with this executable and name them appropriately. The following naming convention is used:

AAA_S####

Where AAA is a file type from the following list:
	- cps
	- evt
	- 2d1
	- 2d2 
	- 2d3
	- 2d4	
and where the #### is the set number of the file. Only EVT files will have a non-zero set number. 

The following is an example for each type of file:
	- cps_S0000
	- evt_S0000
	- evt_S0001
	- evt_S0055
	- 2d1_S0000
	- 2d2_S0000
	- 2d3_S0000
	- 2d4_S0000
	
The following files will be produced by running this script:
	- ..._translated.txt
	- ..._EVT_2DH.txt
where the "..." is replaced by the file that was processed, for example:
	- cps_S0000_translated.txt

For the CPS and EVT data products, the "..._translated.txt" file is a tab separated text file with a header at the top which describes each column. The header is different for each data product type (cps, evt). 
For the 2DH data product, the "..._translated.txt" file is a tab separated array of 2D-histogram bin values. The header at the top describes the binning, briefly.
	
The "...EVT_2DH.txt" file only applies to EVT data product files. It is a recreation of the 2DH data product from one EVT file. It is meant to be used as a comparison to the actual 2DH file produced and is in the same format as the 2DH_translated file.
