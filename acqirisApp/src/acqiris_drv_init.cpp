/* 
 Original Author: Perazzo, Amedeo
 Current Author: Hu, Yong <yhu@bnl.gov>
 */

#include <stdio.h>
#include <string.h>

#include "AcqirisD1Import.h"
#include "AcqirisImport.h"

#include "epicsExport.h"
#include "iocsh.h"
#include "drvSup.h"
#include "epicsPrint.h"
#include "epicsThread.h"

#include "acqiris_drv.hh"
#include "acqiris_daq.hh"

namespace
{
    const char rcs_id[] = "$Id$";
}

//global variable
acqiris_driver_t acqiris_drivers[MAX_DEV];

static int nbr_acqiris_drivers = 0;

//forward function declaration
static void
check_version_number(ViInt32 id);
static int
acqiris_find_devices(int calibration);

//initialize all acqiris digitizers in the crate with calibration option
static int
acqirisInit(int calibration)
{
    acqiris_driver_t* ad;
    int channel;
    char name[32];
    int size;

    nbr_acqiris_drivers = acqiris_find_devices(calibration);
    if (0 == nbr_acqiris_drivers)
    {
        //fprintf(stderr, "*** Could not find any acqiris device\n");
        errlogPrintf("*** Could not find any acqiris device\n");
        return -1;
    }

    for (int module = 0; module < nbr_acqiris_drivers; module++)
    {
        ad = &acqiris_drivers[module];
        /*Cancel the calibration process after initialization
         * so that no periodical cal which might impact digitizing
         */
        //Acqrs_calibrateCancel(ad->id);
        ad->run_semaphore = epicsEventMustCreate(epicsEventEmpty);
        ad->daq_mutex = epicsMutexMustCreate();
        ad->count = 0;
        for (channel = 0; channel < ad->nchannels; channel++)
        {
            size = (ad->maxsamples + ad->extra) * sizeof(short);
            ad->data[channel].nsamples = 0;
            ad->data[channel].buffer = new char[size];
        }

        snprintf(name, sizeof(name), "tacqirisdaq%u", module);
        scanIoInit(&ad->ioscanpvt);
        epicsThreadMustCreate(name, epicsThreadPriorityMin, 5000000,
                acqiris_daq_thread, ad);
        printf("DAQ thread (%s) is created for digitizer #%d\n", name, module);
    } //for (int module=0; module<nbr_acqiris_drivers; module++)

    return (0);
}//static int acqirisInit(int calibration)

static const iocshArg acqirisInitArg0 =
{ "calibration", iocshArgInt };
static const iocshArg * const acqirisInitArgs[1] =
{ &acqirisInitArg0 };
static const iocshFuncDef acqirisInitFuncDef =
{ "acqirisInit", 1, acqirisInitArgs };

static void
acqirisInitCallFunc(const iocshArgBuf *arg)
{
    acqirisInit(arg[0].ival);
}

void
acqirisRegistrar()
{
    iocshRegister(&acqirisInitFuncDef, acqirisInitCallFunc);
}
epicsExportRegistrar(acqirisRegistrar);

//called by acqirisInit()
static int
acqiris_find_devices(int calibration)
{
    ViInt32 nbrInstruments = 0;
    ViString options;
    int module = 0;

    ViStatus status = Acqrs_getNbrInstruments(&nbrInstruments);
    if (VI_SUCCESS != status)
    {
        errlogPrintf("can't get any instrument \n");
        return 0;
    }
    printf("%d modules found \n", (int) nbrInstruments);

    if (0 == calibration)
    {
        options = "cal=0 dma=1";
        printf("No calibration during power-up \n");
    }
    else
    {
        options = "cal=1 dma=1";
        printf("Calibration in progress, Wait... \n");
    }

    for (module = 0; module < nbrInstruments; module++)
    {
        acqiris_driver_t *ad = &acqiris_drivers[module];
        char name[20];
        sprintf(name, "PCI::INSTR%d", module);
        status = Acqrs_InitWithOptions(name, VI_FALSE, VI_FALSE, options,
                (ViSession*) &ad->id);
        if (VI_SUCCESS != status)
        {
            errlogPrintf(
                    "Init failed (%x) for instrument %s with options(%s) \n",
                    (unsigned) status, name, options);
            /*in some cases, one board can't be initialized with calibration,
             * try to re-initialize it without calibration
             */
            //return module;
            //ViString optionsTemp = "cal=0 dma=0";
            status = Acqrs_InitWithOptions(name, VI_FALSE, VI_FALSE,
                    "cal=0 dma=1", (ViSession*) &ad->id);
            if (VI_SUCCESS != status)
            {
                errlogPrintf(
                        "Init failed (%x) again for %s even without calibration\n",
                        (unsigned) status, name);
                return module;
            }
            else
            {
                errlogPrintf(
                        "re-initialized %s without calibration: status:0x%x\n",
                        name, (unsigned) status);
            }
        }
        else
        {
            errlogPrintf(
                    "Initialized module(%s) with options(%s): status:0x%x\n",
                    name, options, (unsigned) status);
        }

        status = Acqrs_getNbrChannels(ad->id, (ViInt32*) &ad->nchannels);
        status = Acqrs_getInstrumentInfo(ad->id, "MaxSamplesPerChannel",
                &ad->maxsamples);
        status = Acqrs_getInstrumentInfo(ad->id, "TbSegmentPad", &ad->extra);
        status = Acqrs_getInstrumentInfo(ad->id, "CrateNb", &ad->CrateNb);
        status = Acqrs_getInstrumentInfo(ad->id, "NbrADCBits", &ad->NbrADCBits);
        status = Acqrs_getInstrumentInfo(ad->id, "PosInCrate", &ad->PosInCrate);
        status = Acqrs_getInstrumentData(ad->id, ad->modelName, &ad->serialNbr,
                &ad->busNbr, &ad->slotNbr);
        if (VI_SUCCESS != status)
        {
            errlogPrintf("can't get more instrument info of %s \n", name);
            return module;
        }

        //ignore check_version_number if the board gives wrong version number
        //check_version_number(ad->id);

        ad->module = module;
    }//for (module=0; module<nbrInstruments; )

    return module;//return the number of successfully initialized boards
}//static int acqiris_find_devices(int calibration)

//ignore check_version_number if the board gives wrong version number
static void
check_version_number(ViInt32 id)
{
    const char *versionstr[] =
    { "Kernel Driver", "EEPROM Common Section", "EEPROM Instrument Section",
            "CPLD Firmware" };
    const ViInt32 expected[4] =
    { 0x10006, 0x3, 0x8, 0x2a };
    ViInt32 found[4];
    for (int v = 0; v < 4; v++)
    {
        Acqrs_getVersion(id, v + 1, found + v);
        if (((found[v] & 0xffff0000) != (expected[v] & 0xffff0000))
                || ((found[v] & 0x0000ffff) < (expected[v] & 0x0000ffff)))
        {
            fprintf(stderr, "Unexpected version 0x%x for %s, expected 0x%x\n",
                    (unsigned) found[v], versionstr[v], (unsigned) expected[v]);
        }
    }//for (int v=0; v<4; v++)
}//static void check_version_number(ViInt32 id)

//for dbior: driver support
static long
report()
{
    acqiris_driver_t* ad;
    printf("********************************************************\n");
    printf("%d digitizers are initialized successfully:\n", nbr_acqiris_drivers);
    for (int module = 0; module < nbr_acqiris_drivers; module++)
    {
        ad = &acqiris_drivers[module];
        printf("digitizer #%d:\n", module);
        printf("\t%d channels, %d samples(max.)/channel, resolution %d-bit;\n",
                ad->nchannels, ad->maxsamples, ad->NbrADCBits);
        printf("\tat slot %d, total %d slots in the crate;\n", ad->PosInCrate,
                ad->CrateNb);
        printf("\tmodel: %s; serial number: %d\n", ad->modelName, ad->serialNbr);
        printf("********************************************************\n");
    }//for (int module=0; module<nbr_acqiris_drivers; module++)

    return 0;
}

static long
init()
{
    acqiris_driver_t* ad;
    printf("********************************************************\n");
    printf("%d digitizers are initialized successfully:\n", nbr_acqiris_drivers);
    for (int module = 0; module < nbr_acqiris_drivers; module++)
    {
        ad = &acqiris_drivers[module];
        printf("digitizer #%d:\n", module);
        printf("\t%d channels, %d samples(max.)/channel, resolution %d-bit;\n",
                ad->nchannels, ad->maxsamples, ad->NbrADCBits);
        printf("\tat slot %d, total %d slots in the crate;\n", ad->PosInCrate,
                ad->CrateNb);
        printf("\tmodel: %s; serial number: %d\n", ad->modelName, ad->serialNbr);
        printf("********************************************************\n");
    }//for (int module=0; module<nbr_acqiris_drivers; module++)

    return 0;
}

static struct
{
    long number;
    DRVSUPFUN report;
    DRVSUPFUN init;
} drvAcqiris =
{ 2, report, init };
epicsExportAddress(drvet,drvAcqiris)
;

