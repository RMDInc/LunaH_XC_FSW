10/18/2018 - Version 2.1/2.2 is the previous iteration of this code and has been used to implement the code which Lee has been contributing. He has worked on many issues within the code including general organization, the configuration file creation, data parsing, modularization of the code into header+source files, and work on the SOH.

- Version 3.0 is level one of the code functionality. We have stripped away interaction in favor of basic function in order to do basic integration with the ASU flat-sat. This code will run initialization processes, then begin looping and report SOH data packets at a rate of 1 Hz. Anything further has been removed for this version. Upcoming iterations will re-integrate interaction.
	
- Version 3.1 has all necessary BOOT files to load the boot image onto an EEPROM and run the system as described above. These files are found in the folder:
		LunaH_XC_FSW/BOOT_Files

- Version 3.2 has updated the BOOT files, as the previous ones did not properly program on the board. Trying a new set of files. Also updated the instructions for loading the program onto the board using Vivado Lab Tools.

- Version 3.21 This update adds a set of BOOT files which are zipped instead of raw in the BOOT_Files folder

- Version 3.22 This update includes a different set of zipped files in the BOOT_Files folder

- Version 3.23 This update properly names the BOOT files within the zip file and updates the instructions for loading a board with the BOOT files.

- Version 3.3 The zip files in the BOOT_Files folder are verified to work and the instructions for loading the board have been updated. 

- Version 3.31 Updated a naming difference in the Loading Instructions.

- Version 3.4 Updated the bitstream for the BOOT files. When programmed with the BOOT files, the board will transmit SOH packets at the higher baud rate of 921600.