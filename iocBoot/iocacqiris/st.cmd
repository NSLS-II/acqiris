#!../acqiris/bin/linux-x86/acqiris
#This startup script file is specifically for NSLS-II Linac Front-end test stand. You should modify this file to make it work for your setup
#the directory '../acqiris' contains the binary file, dbd file and db files

#acqiris at LFE (Linac Front-end) for WCMs (Wall Current Monitor) and Faraday Cup(FC).
epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES","10000000")
epicsEnvSet("ENGINEER","Yong Hu: x3961")
epicsEnvSet("LOCATION","Blg 902 Rm 18")

dbLoadDatabase "../acqiris/dbd/acqiris.dbd"
acqiris_registerRecordDeviceDriver pdbbase

#acqirisInit(int calibration): 0: no calibration during power-up, mostly for quick system startup during tests; 
#			       others: perform calibration during startup for precise measurement
acqirisInit(0)
#acqirisInit(1)

#For the Elma cPCI crate and CR11 cPCI CPU at LFE, the first Acqiris digitizer is at the bottom of the crate, the forth is at the top close to the CPU
#The first digitizer  (MODULE=0, ACQ:1) is DC271: 4-ch, 1GS/s, 8-bit;
#The second digitizer (MODULE=1, ACQ:2) is DC252: 2-ch, 4GS/s, 10-bit;
#The third digitizer  (MODULE=2, ACQ:3) is DC252: 2-ch, 4GS/s, 10-bit;
#The forth digitizer  (MODULE=3, ACQ:4) is DC252: 2-ch, 4GS/s, 10-bit;

#the forth Acqiris is DC252(4GS/s, 10-bit, 2-ch) for 2 Wall Current Monitors
dbLoadRecords ("../acqiris/db/acqiris_channel.db", "PREFIX=LFE-BI{WCM:1},MODULE=3,CHANNEL=0,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_channel.db", "PREFIX=LFE-BI{WCM:2},MODULE=3,CHANNEL=1,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_module.db",  "PREFIX=LFE-BI{ACQ:4},MODULE=3,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_trigger.db", "PREFIX=LFE-BI{ACQ:4},MODULE=3")
#the third Acqiris is DC252(4GS/s, 2-ch) for 2 Faraday Cups
dbLoadRecords ("../acqiris/db/acqiris_channel.db", "PREFIX=LFE-BI{FC:1},MODULE=2,CHANNEL=0,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_channel.db", "PREFIX=LFE-BI{FC:2},MODULE=2,CHANNEL=1,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_module.db",  "PREFIX=LFE-BI{ACQ:3},MODULE=2,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_trigger.db", "PREFIX=LFE-BI{ACQ:3},MODULE=2")
#the second Acqiris is DC252(4GS/s, 2-ch): suppose for other WCMs
dbLoadRecords ("../acqiris/db/acqiris_channel.db", "PREFIX=LFE-BI{WCM:3},MODULE=1,CHANNEL=0,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_channel.db", "PREFIX=LFE-BI{WCM:4},MODULE=1,CHANNEL=1,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_module.db",  "PREFIX=LFE-BI{ACQ:2},MODULE=1,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_trigger.db", "PREFIX=LFE-BI{ACQ:2},MODULE=1")
#the first Acqiris is DC271(1GS/s, 4-ch): suppose for other FCs
dbLoadRecords ("../acqiris/db/acqiris_channel.db", "PREFIX=LFE-BI{FC:3},MODULE=0,CHANNEL=0,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_channel.db", "PREFIX=LFE-BI{FC:4},MODULE=0,CHANNEL=1,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_module.db",  "PREFIX=LFE-BI{ACQ:1},MODULE=0,NELM=4000")
dbLoadRecords ("../acqiris/db/acqiris_trigger.db", "PREFIX=LFE-BI{ACQ:1},MODULE=0")

dbLoadRecords ("../acqiris/db/iocAdminSoft.db", "IOC=LFE-BI{IOC:cPCI}")
dbLoadRecords ("../acqiris/db/save_restoreStatus.db", "P=LFE-BI{AS:cPCI}")

save_restoreSet_status_prefix("LFE-BI{AS:cPCI}")
set_savefile_path("./as", "/save")
set_requestfile_path("./as", "/req")
system("install -d ./as/save")
system("install -d ./as/req")
set_pass0_restoreFile("acqiris_settings.sav")

iocInit

makeAutosaveFileFromDbInfo("./as/req/acqiris_settings.req", "autosaveFields_pass0")
create_monitor_set("acqiris_settings.req", 10 , "")
