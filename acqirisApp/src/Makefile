TOP=../..

include $(TOP)/configure/CONFIG

#=============================
# Build the IOC application

PROD_IOC = acqiris
# acqiris.dbd will be created and installed
DBD += acqiris.dbd

# acqiris.dbd will be made up from these files:
acqiris_DBD += base.dbd
#include dbd file for acqiris device support
acqiris_DBD += acqiris_support.dbd
#include iocStatus and autosave which are installed by epics Debian package
# /usr/lib/epics/dbd/
acqiris_DBD += devIocStats.dbd
acqiris_DBD += asSupport.dbd
acqiris_DBD += system.dbd

# acqiris_registerRecordDeviceDriver.cpp derives from acqiris.dbd
acqiris_SRCS += acqiris_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
acqiris_SRCS_DEFAULT += acqirisMain.cpp
acqiris_SRCS_vxWorks += -nil-

# add acqiris driver/device codes
acqiris_SRCS += acqiris_dev_li.cpp
acqiris_SRCS += acqiris_dev_lo.cpp
acqiris_SRCS += acqiris_dev_ai.cpp
acqiris_SRCS += acqiris_dev_ao.cpp
acqiris_SRCS += acqiris_dev_mbbo.cpp
acqiris_SRCS += acqiris_dev_wf.cpp
acqiris_SRCS += acqiris_dev_init.cpp
acqiris_SRCS += acqiris_drv_li.cpp
acqiris_SRCS += acqiris_drv_lo.cpp
acqiris_SRCS += acqiris_drv_ai.cpp
acqiris_SRCS += acqiris_drv_ao.cpp
acqiris_SRCS += acqiris_drv_mbbo.cpp
acqiris_SRCS += acqiris_drv_wf.cpp
acqiris_SRCS += acqiris_daq.cpp
acqiris_SRCS += acqiris_drv_init.cpp
acqiris_SRCS += acqiris_asub_process.cpp

#use aSubRecord.h(contains 'not') in .cpp file
CXXFLAGS+=-fno-operator-names  
USR_CFLAGS += -I/usr/include -D_ACQIRIS -D_LINUX
USR_CPPFLAGS += -I/usr/include -D_ACQIRIS -D_LINUX
acqiris_SYS_LIBS += AqDrv4
#include autosave and devIocStats libraries which are installed by epics Debian
#package: /usr/lib/epics/lib/linux-x86
acqiris_LIBS += autosave
acqiris_LIBS += devIocStats 

#this method doesn't work well for dynamic/shared library(libAqDrv4.so)
#acqiris_LIBS += acqdrv

# Finally link to the EPICS Base libraries
acqiris_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

