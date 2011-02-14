#!../../bin/linux-x86/acqiris

epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES","10000000")
epicsEnvSet("ENGINEER","Yong Hu: x3961")
epicsEnvSet("LOCATION","Blg 902 Rm 18")

cd ../..

dbLoadDatabase "dbd/acqiris.dbd"
acqiris_registerRecordDeviceDriver pdbbase

acqirisInit(1)

dbLoadTemplate "db/acqiris_channel.substitutions"
dbLoadTemplate "db/acqiris_module.substitutions"
dbLoadTemplate "db/acqiris_trigger.substitutions"
dbLoadRecords "db/iocStats.db", "IOCNAME=LBT:BI:Acqiris"

set_savefile_path("./autosave")
set_requestfile_path("./autosave")
set_pass0_restoreFile("acqiris_settings.sav")
set_pass1_restoreFile("acqiris_settings.sav")

iocInit

makeAutosaveFileFromDbInfo("acqiris_settings.req", "autosaveFields_pass0")
makeAutosaveFileFromDbInfo("acqiris_settings.req", "autosaveFields_pass1")
create_monitor_set("acqiris_settings.req", 10 , "Psy=LBT, Ssy=BI, Dev=[FCT:1:2]")
