/* 
    Original Author: Perazzo, Amedeo
    Modified by Yong Hu: 10/29/2010
*/

#ifndef ACQIRIS_DRV_HH
#define ACQIRIS_DRV_HH

#define MAX_CHANNEL 4
#define MAX_DEV 10

#include <dbScan.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsThread.h>

//#include <AcqirisD1Import.h>
//#include <AcqirisImport.h>

struct acqiris_data_t {
  unsigned nsamples;
  void* buffer;
};

struct acqiris_driver_t {
  int module;
  int nchannels;
  int id;
  IOSCANPVT ioscanpvt;
  epicsEventId run_semaphore;
  epicsMutexId daq_mutex;
  int running;
  int extra;
  int maxsamples;
  unsigned count;
  unsigned timeouts;
  unsigned readerrors;
  unsigned truncated;
  acqiris_data_t data[MAX_CHANNEL];
//Yong Hu: get more instrument information
  int CrateNb;
  int NbrADCBits;
  int PosInCrate;
  char *VersionUserDriver;
  //char *modelName;
  char modelName[32];
  long serialNbr;
  long busNbr;
  long slotNbr;
};
typedef struct acqiris_driver_t ad_t;


/*
***must use 'extern' qualifier to /declare/ these global variables;
***if no 'extern', when multiple souce files include this header file, these variables will be defined serveral times
and we'll get multiple definitions errors;  
***Global variables should be defined only once, but they can have multiple declarations.
*/

//Yong Hu
/*
#ifdef __cplusplus 
extern "C" { 
#endif
*/

extern "C" 
{
extern acqiris_driver_t acqiris_drivers[];
//acqiris_driver_t acqiris_drivers[MAX_DEV];
extern unsigned nbr_acqiris_drivers;
//unsigned nbr_acqiris_drivers;
extern epicsMutexId acqiris_dma_mutex;
//epicsMutexId acqiris_dma_mutex;
extern long timeout;//yhu: acquisition timeout
}

/*
#ifdef __cplusplus 
} 
#endif
*/

#define SUCCESS(x) (((x)&0x80000000) == 0)

#endif
