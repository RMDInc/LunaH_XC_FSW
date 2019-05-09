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
When finished, the Level 2 FSW will be running on an XC board. At that point, a test macro and binary reader/packet reader will be used to test the implementation and ensure that everything is worked as intended. The date for that to be finished is projected to be 11/16/2018.

- Version 3.61 James updated the code and added the reportSuccesss/Failure commands to the utils file. These functions allow us to create a command SUCCESS/FAILURE packet when a function needs to report such. 

- Version 3.62 Graham merged James' code into the repository. Also added was a folder for TeraTerm macros which is in the commands/ folder. Each update will have a spot for the macro for that FSW level.

- Version 3.621 Transition commit.

- Version 3.63 Successfully merged the new code into the repository.

- Version 3.7 This update adds the full Level 2 packet output to the FSW. Each of the functions for Level 2 have been checked for functionality and testing and validation with a macro and binary reader are beginning. Once validated, this version of the FSW will be shipped. 

- Version 3.71 Updated the macro for grabbing the packets that the FSW creates and pushes to the UART. Also have an early version of the application for processing and recognizing the packets in a binary file. 

- Version 3.8 The level 2 FSW code is not updated with this version, but the macros for running and testing the system are included, as well as a test binary output file with CCSDS packets in it. A future update will include instructions for running all of L2.

- Version 3.81 Level 2 FSW BOOT files are updated and can be found in the BOOT_Files folder under the name XC_FSW_L2.zip. Unzip the files to find the fsbl, bootimage file, and .mcs file. Instructions for programming an engineering board with these files to run the L2 FSW will come with a later update.

- Version 4.0 This version number indicates the start of Level 3 FSW development. While at LANL, work was started on the L3 FSW in a number of areas: the command names are in the process of being updated, DAQ is being implemented, 
the DAQ source file is being developed, the DAQ code is in reference to the "fake data" program that was written to generate LunaH data products and data packets, much of the code has been cleaned up with regards to various left over 
variables and define values, various references within the code to instances of the HW are being passed now, especially in the SOH generation, plus a lot of the functions and variables have been re-organized for clarity and usefullness.
Also, I have begun to format the information for each function within the source files so that someone could look at the text comments and figure out how to use the function, what its purpose is, and what its parameters and return value 
are. This should add clarity and begins the start of a style guide for how to add to this project. This version of the FSW is begins the third release of the FSW. In terms of the L2 FSW, I need to finish writing the instructions and maybe publish a new ICD version.

- Version 4.1 This version has fixed a lot of bugs with reading in user commands and has added a lot of support for creating packets and accessing data which is scanned in by other functions. I also begin to handle DAQ with this verison.
Pre-DAQ and DAQ init are begun as well as handling reporting command success/failure.

- Version 4.2 Deleted a lot of unnecessary code and old code to keep things tidy. Merged the NGATES and NWGATES functions, keeping just NGATES. This is because we only need one neutron cut access functions. We are moving to elliptical cuts for the neutron cuts, so the configuration file, NGATES function have been updated to reflect the change. Taking a look at the init code near the top of main to verify that it's useful and we need to keep it. Beginning to put the structure of DAQ into place; most of the loops are established. James is working on the TX function, but that work is not reflected here.

- Version 4.3 Large update including SD card work, CPS code for DAQ, the beginnings of the rest of the DAQ framework, and some testing files. Have done testing with the SD card timing and organizational structure. 

- Version 4.39 This update is the code update for the Level 2 FSW. Minor changes and bug fixes were made to finalize the code which goes into this release. 
The next update will be the L2 FSW BOOT files and the update after that will contain the instructions for programming a board with the software, testing with the provided TeraTerm macro, 
running the Packet Reader on the output packets, a validation spreadsheet, and the telemetry dictionary. The dictionary may be distributed later than the next few updates due to complexity. 

- Version 4.40 This update includes the BOOT files for the Mini-NS Level 2 FSW. A minor change has also been added to one of the Mini-NS payload commands; it has been documented in the ICD accordingly. 

- Version 4.41 Added instructions and programs for testing the L2 FSW.  

- Version 4.42 Full release of L2 FSW. The instructions for running the Macro Test have been updated, as has the executable which tests the output packets from the run. 

- Version 4.43 Deleted unnecessary files from the repository.

- Version 5.0 First update post-L2 FSW. This software is now moving towards running DAQ and WF to collect data while on the lab bench. Lots of code has been added to initialize, error check, and otherwise support data acquisition operations. This version has not been tested in the lab yet. 

- Version 5.1 Investingating issue with the bitstream and clearing the buffers, performing DMA transfers. 

- Version 5.2 ASU update number 1. This has DAQ which saves EVTs data product to the SD card, can TX files from the SD card, can collect AA waveforms. Changed the verison of the Zynq document in the folder. Added a new test macro to L3 folder.

- Version 5.3 Post-ASU trip update. I have updated the code with the patch to SOH packet creation, so the checksums are now calculated correctly. Further, SOH packets are updated for V3 and all versions going forward with FIXED field sizes for all information reported. This will allow packets to be displayed when we are reading them from AIT and COSMOS. This version has an unidentified bug within DAQ which causes multiple columns & rows to not be displayed/sorted when creating the 2DH during and at the end of a run. Some testing code is currently in this version within DAQ, process data, main source files. To test the new SOH packetization, the V1 and V2 macros were updated. Graham has test packet readers on the K drive at RMD.

- Version 5.4 Re-organized the development for the FSW and i'm finishing up version 3a at the moment. Currently, DAQ is working as intended, but without proper header/footer information tacked on. There are headers and footers on all files, but they are not optimal for what I want to implement, see notes p.29, book 12. Working on transfer code for version 4a at the moment and it's in the planning stages, see p. 33, book 12. This explains my method and how I'll be writing the code. For DAQ, I have added mid-DAQ file creation. For general system information, I have added a folder/file structure to the SD card storage of files. Each DAQ run will have a folder created where all the run files will be stored. I've changed a lot about how files are created and how the header/footer information is collected and written in. Added the TX code to the project in lunah_utils source file. All data product files are produced, as per the ICD. TX function isn't fully packetized though. Added a placeholder for the neutron tallying for the CPS data product. Cuts are applied the old way of using square cuts and then tallying wihtin a 2D array. Fixed fields have been implemented in most output packets thus far. SOH and report success/failure packets are the main ones, though. The BOOT files from the ASU trip have been added to the BOOT_Files folder in this repository. The source code is stored in a different place, ask Graham if there are questions with that.

- Version 5.41 Updating the source code as we believe we have figured out how to get false events for each DAQ run. This is a pre-commit to preserve any code updates before we change the bitstream. Meg has had a new bitstream for a while, but it's now being integrated into the main project. This will be the primary XC bitstream for the time being. This bitstream is named "adc_com_116" in Meg's shared drive. Further commits later will bring that in.

- Version 5.42 The bitstream update from the previuos update (5.41) was successful and the current bitream in this project will be the one going forward. The FSBL and it's BSP have been removed from the project. They will likely be reintroduced with the version 3 release. Major code rework is almost in place for the transfer function. The transfer function will eventually be able to send all types of packets and files on the SD card with proper formatting. All packets of a specific type will have the same size. By ensuring that packets are the same size, we can make sure that AIT and Cosmos can read packets consistently to display their contents. Things to note about this update: the RMD data header in the ICD is not what is reported in those bytes at the moment because we're transitioning between different kinds of neutron cuts, the the values that are actually in the data header are the values for PMT 1 only, headers and footers are properly implemented in the data files on the SD card when created during DAQ. There may be other caveats for this update not mentioned here. This update also has not been tested on a board with an analog. That is coming in the next few days and there will be another update to confirm that, as well as fix any bugs which were introduced with this update. 

- Version 5.43 The transfer function for the CPS, EVT, and 2DH data products is committed with this update. They have been through a first round of testing, but need to be tested a bit further before being considered fully operational. Further, the CFG, LOG, and LS functions are not working yet, as they are a much lower priority for the moment. The CPS tallies during DAQ have been implemented with this update, so neutron counts will now be present within the CPS data product. The cuts implemented are the baseline "box" cuts where we cut according to a constant energy or PSD value which comes from either the default value (hard coded) or as a user-defined value submitted via the SET_NGATES command. To support the "box" neutron cuts, there is various code which is lying around with notes to remove it once we move to the more complex elliptical neutron cuts which are temperature dependent. These notes and the "box" cut code will be removed at that time. It seems that SPI code has been added to the bitstream and brought into the project via the BSP during this update. The telemetry dictionary has also been updated very slightly, but doesn't have most of the information for packets that is necessary. A future update will bring this information. If that is needed, contact Graham.

- Version 5.50 Some changes were made to accomodate the HSFL FSW for this update. Those changes are temporary to allow them to exercise things like DAQ and other commands without having an analog board connected to handle IIC stuff. The FSBL and FSBL_BSP were re-introduced for this update so that I could create the BOOT files for the upcoming HSFL test. A new set of boot files has been produced and placed in a zip file in the BOOT_Files folder under Neutron-1 XC FSW V3. The procedure to program is the same as before.