To run the Packet Reader over the L2 packets which are produced with the L2_XC_FSW_TEST_MACRO.ttl Tera Term macro, collect a binary TT log and place it in this file. 
When finished running the L2 macro, power off the board and remove the SD card to retrieve the MNSCONF.bin file which contains the current system configuration.
Place both files into this folder so the Packet Reader executable can find them and process them.
They must be named:

L2_MACRO_OUT.bin
MNSCONF.bin

The following files will be produced:

L2_config_analyze.txt
L2_packets_noSOH_vals.txt
L2_packets_raw_vals.txt
L2_packets_read.txt

Each file contains a slightly different take on what was in the packets which were logged:
	- The config analyze file compares the current system configuration to an exprected set of values.
	- The packets noSOH file only displays packets which were read which did not have an SOH APID value.
	- The packets raw vals file just shows the APID of each packet
	- The packets read file shows the APID and stated length of each packet.