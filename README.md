10/18/2018 - Version 2.1/2.2 is the previous iteration of this code and has been used to implement the code which Lee has been contributing. He has worked on many issues within the code including general organization, the configuration file creation, data parsing, modularization of the code into header+source files, and work on the SOH.

	- Version 3.0 is level one of the code functionality. We have stripped away interaction in favor of basic function in order to do basic integration with the ASU flat-sat. This code will run initialization processes, then begin looping and report SOH data packets at a rate of 1 Hz. Anything further has been removed for this version. Upcoming iterations will re-integrate interaction.
	
	- Version 3.1 has all necessary BOOT files to load the boot image onto an EEPROM and run the system as described above. These files are found in the folder:
		LunaH_XC_FSW/BOOT_Files
