#!../../bin/linux-x86/acqiris

epicsEnvSet("AcqirisDxDir","/usr/local/AcqirisLinux/demo")
epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES","10000000")
epicsEnvSet("ENGINEER","Yong Hu: x3961")
epicsEnvSet("LOCATION","Blg 902 Rm 18")
epicsEnvSet "EPICS_CAS_AUTO_BEACON_ADDR_LIST", "NO"
epicsEnvSet "EPICS_CAS_BEACON_ADDR_LIST", "130.199.240.255 130.199.241.255:5065"
#epicsEnvSet "EPICS_CAS_BEACON_ADDR_LIST", "130.199.195.255:5065"
epicsEnvSet "EPICS_CAS_SERVER_PORT", "8001" 

cd ../..

dbLoadDatabase "dbd/acqiris.dbd"
acqiris_registerRecordDeviceDriver pdbbase

acqirisInit(1)

dbLoadRecords "db/acqiris_channel.db"
dbLoadRecords "db/acqiris_module.db"
dbLoadRecords "db/acqiris_trigger.db"
dbLoadRecords ("db/iocAdminSoft.db", "IOC=BTS-BI{WCM}")
dbLoadRecords ("db/save_restoreStatus.db", "P=BTS-BI{WCM}")

save_restoreSet_status_prefix("BTS-BI{WCM}")
set_savefile_path("./as", "/save")
set_requestfile_path("./as", "/req")
system("install -d ./as/save")
system("install -d ./as/req")
set_pass0_restoreFile("acqiris_settings.sav")
#set_pass1_restoreFile("acqiris_settings.sav")

iocInit

makeAutosaveFileFromDbInfo("as/req/acqiris_settings.req", "autosaveFields_pass0")
#makeAutosaveFileFromDbInfo("acqiris_settings.req", "autosaveFields_pass1")
create_monitor_set("acqiris_settings.req", 10 , "")
