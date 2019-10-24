To run the Packet Reader over the V4 packets which are produced with the V4 Checkout Det 0-DAQ/WF ttl macro Tera Term macro, collect a binary TT log and place it in this folder.
When finished running the V4 macro, power off the board.
If you are going to analyze the transfer command, remove the SD card from the board and retrieve the data product files.
Place all files into this folder so the V4 Output Packet Reader executable can find them and process them.

A number of files will be produced equal to the number of MNS_TX commands that were sent plus one. Each MNS_TX transfers one data product file and the Reader will process each packet and place all of the data products for each transfer in a separate file. The last file is a "..._PACKETS.txt" file which is a review of the bytes in each output packet including things like the checksums and whether they were verified.
