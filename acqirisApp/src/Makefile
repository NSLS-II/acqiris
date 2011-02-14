TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#Yong Hu:this method doesn't work well for dynamic/shared library: firstly don't add 'acqiris_LIBS += AqDrv4'
#in order to install libAqDrv4.so, then add 'acqiris_LIBS += AqDrv4' as below
#must add CFLAGS and CPPFLAGS
#INC += AcqirisD1Import.h AcqirisDataTypes.h AcqirisExDataTypes.h AcqirisExInterface.h AcqirisG4Interface.h AcqirisInterface.h AcqirisT3Interface.h visa.h vpptype.h AcqirisD1Interface.h AcqirisErrorCodes.h AcqirisExImport.h AcqirisG4Import.h AcqirisImport.h AcqirisT3Import.h ivi.h visatype.h
#LIB_INSTALLS_Linux += ../libAqDrv4.so
#USR_CFLAGS += -D_ACQIRIS -D_LINUX
#USR_CPPFLAGS += -D_ACQIRIS -D_LINUX

#Yong Hu:static library libacqdrv.a is built by RTEMS, not applicable here. 
#INC += AcqirisD1Import.h AcqirisDataTypes.h AcqirisExDataTypes.h AcqirisExInterface.h AcqirisG4Interface.h AcqirisInterface.h AcqirisT3Interface.h visa.h vpptype.h AcqirisD1Interface.h AcqirisErrorCodes.h AcqirisExImport.h AcqirisG4Import.h AcqirisImport.h AcqirisT3Import.h ivi.h visatype.h
#LIB_INSTALLS_Linux += ../libacqdrv.a
#USR_CFLAGS += -D_ACQIRIS -D_LINUX
#USR_CPPFLAGS += -D_ACQIRIS -D_LINUX

#=============================
# Build the IOC support library

LIBRARY_IOC += acqirisSupport

# xxxRecord.h will be created from xxxRecord.dbd
DBDINC += xxxRecord
# Install devXxxSoft.dbd into <top>/dbd
DBD += xxxSupport.dbd

# Compile and add the code to the support library
acqirisSupport_SRCS += xxxRecord.c
acqirisSupport_SRCS += devXxxSoft.c

# Link locally-provided code into the support library,
# rather than directly into the IOC application.
acqirisSupport_SRCS += dbSubExample.c
acqirisSupport_SRCS += acqirisHello.c
acqirisSupport_SRCS += initTrace.c

acqirisSupport_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
# Build the IOC application

PROD_IOC = acqiris
# acqiris.dbd will be created and installed
DBD += acqiris.dbd

# acqiris.dbd will be made up from these files:
acqiris_DBD += base.dbd
acqiris_DBD += xxxSupport.dbd
acqiris_DBD += dbSubExample.dbd
acqiris_DBD += acqirisHello.dbd
acqiris_DBD += initTrace.dbd
#Yong Hu:include dbd file for acqiris device support
acqiris_DBD += acqiris_support.dbd
#Yong Hu: include iocStatus and autosave which are installed by epics Debian package
# /usr/lib/epics/dbd/
acqiris_DBD += devIocStats.dbd
acqiris_DBD += asSupport.dbd

# acqiris_registerRecordDeviceDriver.cpp derives from acqiris.dbd
acqiris_SRCS += acqiris_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
acqiris_SRCS_DEFAULT += acqirisMain.cpp
acqiris_SRCS_vxWorks += -nil-

#Yong Hu: add acqiris driver/device codes
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

USR_CFLAGS += -I/usr/include -D_ACQIRIS -D_LINUX
USR_CPPFLAGS += -I/usr/include -D_ACQIRIS -D_LINUX
acqiris_LDFLAGS += -L/usr/lib -lAqDrv4
#Yong Hu:include autosave and devIocStats libraries which are installed by epics Debian
#package: /usr/lib/epics/lib/linux-x86
acqiris_LIBS += autosave
acqiris_LIBS += devIocStats 

#Yong Hu:this method doesn't work well for dynamic/shared library(libAqDrv4.so)
#see notes above
#acqiris_LIBS += acqdrv

# Add support from base/src/vxWorks if needed
#acqiris_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Link in the code from the support library
acqiris_LIBS += acqirisSupport

# NOTE: To build SNL programs, SNCSEQ must be defined
# in the <top>/configure/RELEASE file

ifneq ($(SNCSEQ),)
    # Build sncExample into acqirisSupport
    sncExample_SNCFLAGS += +r
    acqiris_DBD += sncExample.dbd
    acqirisSupport_SRCS += sncExample.stt
    acqirisSupport_LIBS += seq pv
    acqiris_LIBS += seq pv

    # Build sncProgram as a standalone program
    PROD_HOST += sncProgram
    sncProgram_SNCFLAGS += +m
    sncProgram_SRCS += sncProgram.st
    sncProgram_LIBS += seq pv
    sncProgram_LIBS += $(EPICS_BASE_HOST_LIBS)
endif

# Finally link to the EPICS Base libraries
acqiris_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

