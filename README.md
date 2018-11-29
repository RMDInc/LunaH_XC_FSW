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

- Version 3.5 This update concerns the source code only; the BOOT files are left as in previous releases. This update changes the scope of the UART access. This should be a more stable version for testing and integration of features by all.

- Version 3.6 This is close to finished Level 2 FSW. This version introduces basic functionality and user interaction. The board will report 4 types of output packets. A lot of reorganization has taken place in terms of breaking out code into modules (header + source file), as well as adding in getter/setter functions for status variables. The Log File is now in place and working. The Receive buffer functionality, which holds commands from the UART until they are processed, has been updated and is now implemented as a 'rolling receive'. This means that as commands fill the buffer they are processed, then deleted from the buffer, and the buffer is shifted over, so commands which came after it are then read, processed, and shifted out. 
When finished, the Level 2 FSW will be running on an XC board. At that point, a test macro and binary reader/packet reader will be used to test the implementation and ensure that everything is worked as intended. The date for that to be finsihed is projected to be 11/16/2018.

- Version 3.61 James updated the code and added the reportSuccesss/Failure commands to the utils file. These functions allow us to create a command SUCCESS/FAILURE packet when a function needs to report such. 

- Version 3.62 Graham merged James' code into the repository. Also added was a folder for TeraTerm macros which is in the commands/ folder. Each update will have a spot for the macro for that FSW level.

- Version 3.621 Transition commit.

- Version 3.63 Successfully merged the new code into the repository.

- Version 3.7 This update adds the full Level 2 packet output to the FSW. Each of the functions for Level 2 have been checked for functionality and testing and validation with a macro and binary reader are beginning. Once validated, this version of the FSW will be shipped. 

- Version 3.71 Updated the macro for grabbing the packets that the FSW creates and pushes to the UART. Also have an early version of the application for processing and recognizing the packets in a binary file. 

- Version 3.8 The level 2 FSW code is not updated with this version, but the macros for running and testing the system are included, as well as a test binary output file with CCSDS packets in it. A future update will include instructions for running all of L2.