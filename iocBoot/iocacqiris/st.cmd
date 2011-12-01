#!../../bin/linux-x86/acqiris
#This startup script is for testing NSLS-II Linac(LN) Wall Current Monitor(WCM)
#in Beam Instrumentation(BI) system. 
#You may need to  modify this file to make it work for your setup; 
#if something wrong happens, check your directory path/structure first

epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES","10000000")
#epicsEnvSet("ENGINEER","Yong Hu: x3961")
#epicsEnvSet("LOCATION","Blg 902 Rm 18")

cd ../..

dbLoadDatabase "dbd/acqiris.dbd"
acqiris_registerRecordDeviceDriver pdbbase

#acqirisInit(int calibration): 
#0: no calibration during power-up, for quick system startup during tests; 
# others: perform calibration during startup for precise measurement
acqirisInit(0)
#acqirisInit(1)

#For the Elma cPCI crate and CR11 cPCI CPU at NSLS-II Linac, 
#the first Acqiris digitizer is at the bottom of the crate
#the first channel of the first Acqiris digitizer: MODULE=0, CHANNEL=0
dbLoadRecords ("db/acqiris_channel.db", "PREFIX=LN-BI{WCM:1},MODULE=0,CHANNEL=0,NELM=4000")
dbLoadRecords ("db/acqiris_module.db",  "PREFIX=LN-BI{ACQ:1},MODULE=0,NELM=4000")
dbLoadRecords ("db/acqiris_trigger.db", "PREFIX=LN-BI{ACQ:1},MODULE=0")

#uncomment the following if you want iocStats and autosave stuff
#dbLoadRecords ("db/iocAdminSoft.db", "IOC=LN-BI{IOC:cPCI}")
#dbLoadRecords ("db/save_restoreStatus.db", "P=LN-BI{AS:cPCI}")

#save_restoreSet_status_prefix("LN-BI{AS:cPCI}")
#set_savefile_path("./as", "/save")
#set_requestfile_path("./as", "/req")
#system("install -d ./as/save")
#system("install -d ./as/req")
#set_pass0_restoreFile("acqiris_settings.sav")
#set_pass1_restoreFile("acqiris_set_pass1.sav")

iocInit

#makeAutosaveFileFromDbInfo("./as/req/acqiris_settings.req", "autosaveFields_pass0")
#create_monitor_set("acqiris_settings.req", 10 , "")
#makeAutosaveFileFromDbInfo("./as/req/acqiris_set_pass1.req", "autosaveFields_pass1")
#create_monitor_set("acqiris_set_pass1.req", 10 , "")

#Need to process the record below again after iocInit to get Max. 8GS/s
#dbpf "LFE-BI{ACQ:4}SampInterval-SP.PROC", "1"
