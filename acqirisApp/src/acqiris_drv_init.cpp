/* 
    Original Author: Perazzo, Amedeo
    Modified by Yong Hu: 10/29/2010
*/

#include "acqiris_drv.hh"
#include "acqiris_daq.hh"

#include <AcqirisD1Import.h>
#include <AcqirisImport.h>

#include <epicsExport.h>
#include <iocsh.h>
//Yong Hu
#include <drvSup.h>

#include <stdio.h>
#include <string.h>

//Yong Hu
/*
#ifdef __cplusplus 
extern "C" { 
#endif 
*/

/*
***define global variables here and only once;
***if no 'extern "C"' is used to declare global variables in 'acqiris_drv.hh',
move these definitions out of 'extern "C"' below;
***if 'extern "C"' is used to declare global variables in 'acqiris_drv.hh',
these variables can be defined anywhere.
*/

//acqiris_driver_t acqiris_drivers[MAX_DEV];
//unsigned nbr_acqiris_drivers = 0;
//epicsMutexId acqiris_dma_mutex;

extern "C" {
  acqiris_driver_t acqiris_drivers[MAX_DEV];
  unsigned nbr_acqiris_drivers = 0;
  epicsMutexId acqiris_dma_mutex;

//Yong Hu: our DC252 version is 0x7, igore check_version_number 
  static void check_version_number(ViInt32 id)
  {
    const char* versionstr[] = {
      "Kernel Driver",
      "EEPROM Common Section",
      "EEPROM Instrument Section",
      "CPLD Firmware"
    };
    const ViInt32 expected[4] = {0x10006, 0x3, 0x8, 0x2a};
    ViInt32 found[4];
    for (int v=0; v<4; v++) {
      Acqrs_getVersion(id, v+1, found+v);
      if (((found[v] & 0xffff0000) != (expected[v] & 0xffff0000)) ||
	  ((found[v] & 0x0000ffff) <  (expected[v] & 0x0000ffff))) {
	fprintf(stderr, "*** Unexpected version 0x%x for %s, expected 0x%x\n",
		(unsigned)found[v], versionstr[v], (unsigned)expected[v]);
      }
    }
  }

  static int acqiris_find_devices(int calibration)
  {
    ViInt32 nbrInstruments;
    ViString options;
    ViStatus status = Acqrs_getNbrInstruments(&nbrInstruments);
    printf("%d modules found \n",(int)nbrInstruments);
    if (0 ==  calibration)
    {
    	options = "cal=0 dma=0";
        printf("No calibration during power-up \n");;
    }
    else
    {
    	options = "cal=1 dma=0";
        printf("Calibration in progress, Wait... \n");;
    }

    int module;
    for (module = 0; module < nbrInstruments; )
    {
    	acqiris_driver_t* ad = &acqiris_drivers[module];
//Yong Hu: pass calibration parameters
      //ViString options = "";
      //ViString options = "cal=0 dma=0";
      //ViString options = "cal=1,dma=1";
     
      char name[20];
      sprintf(name, "PCI::INSTR%d", module);
      status = Acqrs_InitWithOptions(name, VI_FALSE, VI_FALSE, options, (ViSession*)&ad->id);
      //Yong Hu
      if (status != VI_SUCCESS)
      {
    	  fprintf(stderr, "*** Init failed (%x) for instrument %s with options(%s) \n",(unsigned)status, name, options);
    	  //continue;
    	  return module;
      }
      printf("Initialized module(%s) with options(%s) successfully(status:0x%x)\n",name,options,(unsigned)status);

//Yong Hu
     // Acqrs_calibrate(ad->id);
     // Acqrs_calibrateEx(ad->id, 0, 0, 0);

      Acqrs_getNbrChannels(ad->id, (ViInt32*)&ad->nchannels);
      Acqrs_getInstrumentInfo(ad->id, "MaxSamplesPerChannel", &ad->maxsamples);
      Acqrs_getInstrumentInfo(ad->id, "TbSegmentPad", &ad->extra);
//Yong Hu: get more instrument information
      Acqrs_getInstrumentInfo(ad->id, "CrateNb", &ad->CrateNb);
      Acqrs_getInstrumentInfo(ad->id, "NbrADCBits", &ad->NbrADCBits);
      Acqrs_getInstrumentInfo(ad->id, "PosInCrate", &ad->PosInCrate);
      Acqrs_getInstrumentInfo(ad->id, "VersionUserDriver", &ad->VersionUserDriver);
      //printf("digitizer version: %s\n", &ad->VersionUserDriver);

//Yong Hu: our DC252 version is 0x7, igore check_version_number 
      //check_version_number(ad->id);
      ad->module = module;
      module++;
    }//for (module=0; module<nbrInstruments; )

    return module;
  }   

  //static int acqirisInit(int order)
  static int acqirisInit(int calibration)
  {
//Yong Hu
    acqiris_driver_t* ad;

    nbr_acqiris_drivers = acqiris_find_devices(calibration);
    if (!nbr_acqiris_drivers) {
      fprintf(stderr, "*** Could not find any acqiris device\n");
      return -1;
    }
    acqiris_dma_mutex = epicsMutexMustCreate();
    for (unsigned module=0; module<nbr_acqiris_drivers; module++) {
      int channel;
      char name[32];
//Yong Hu: Cancel the calibration process after initialization 
//so that no periodical cal which might impact digitzing
      ad = &acqiris_drivers[module];
      Acqrs_calibrateCancel(ad->id);
      //printf("Cancel the calibration process after initialization--yhu\n");

      ad->run_semaphore = epicsEventMustCreate(epicsEventEmpty);
      ad->daq_mutex = epicsMutexMustCreate();
      ad->count = 0;
      for(channel=0; channel<ad->nchannels; channel++) {
	int size = (ad->maxsamples+ad->extra)*sizeof(short);
	ad->data[channel].nsamples = 0;
	ad->data[channel].buffer = new char[size];
      }
      snprintf(name, sizeof(name), "tacqirisdaq%u", module);
      scanIoInit(&ad->ioscanpvt);      //printf("digitizer version: %s\n", &ad->VersionUserDriver);
      epicsThreadMustCreate(name, 
			    epicsThreadPriorityMin, 
			    5000000,
			    acqiris_daq_thread, 
			    ad); 
    } //for (unsigned module=0; module<nbr_acqiris_drivers; module++)  
  
    //Yong Hu
    //Acqrs_calibrateCancel(ad->id);
    //printf("Cancel the calibration process after initialization--yhu\n");
    return 0;
  }

  static const iocshArg acqirisInitArg0 = {"calibration",iocshArgInt};
  static const iocshArg * const acqirisInitArgs[1] = {&acqirisInitArg0};
  static const iocshFuncDef acqirisInitFuncDef =
    {"acqirisInit",1,acqirisInitArgs};

  static void acqirisInitCallFunc(const iocshArgBuf *arg)
  {
    acqirisInit(arg[0].ival);  
  }

  void acqirisRegistrar()
  {
    iocshRegister(&acqirisInitFuncDef,acqirisInitCallFunc);
  }

  epicsExportRegistrar(acqirisRegistrar);

//Yong Hu:for dbior()
//wrong if "static long report(int level)"
  static long report()
  {
    int module;
    acqiris_driver_t* ad;
    //printf("dbior: testing --yhu\n");
    printf("********************************************************\n");
    printf("%d modules/digitizers are initialized successfully:\n", nbr_acqiris_drivers);
    for (module=0; module<nbr_acqiris_drivers; module++) 
    {
      ad = &acqiris_drivers[module];
      printf("digitizer #%d:\n", module);
//must use "&ad->VersionUserDriver"
      printf("	User driver version: %s;\n", &ad->VersionUserDriver);
      printf("	%d channels, %d samples(max.)/channel, resolution %d-bit;\n", ad->nchannels, ad->maxsamples, ad->NbrADCBits);
      printf("	at slot %d, total %d slots in the crate;\n", ad->PosInCrate, ad->CrateNb);
      printf("********************************************************\n");
    }

    return 0;
  }

  static long init()
  {
    int module;
    acqiris_driver_t* ad;
    //printf("dbior: testing --yhu\n");
    printf("********************************************************\n");
    printf("%d modules/digitizers are initialized successfully: \n", nbr_acqiris_drivers);
    for (module=0; module<nbr_acqiris_drivers; module++) 
    {
      ad = &acqiris_drivers[module];
      printf("digitizer #%d:\n", module);
//must use "&ad->VersionUserDriver"
      printf("	User driver version: %s;\n", &ad->VersionUserDriver);
      printf("	%d channels, %dK samples(max.)/channel, resolution %d-bit;\n", ad->nchannels, ad->maxsamples / 1024, ad->NbrADCBits);
      printf("	at slot %d, total %d slots in the crate;\n", ad->PosInCrate, ad->CrateNb);
      printf("********************************************************\n");
    }
    return 0;
  }

  struct {
    long number;
    DRVSUPFUN report;
    DRVSUPFUN init;
    } drvAcqiris = {
      2,
      report,
      init
    };

  epicsExportAddress(drvet,drvAcqiris);

} //extern "C"

/*
#ifdef __cplusplus 
} 
#endif 
*/

