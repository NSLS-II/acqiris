/* 
 Original Author: Perazzo, Amedeo
 Current Author: Hu, Yong <yhu@bnl.gov>
 $Id$
 */

#ifndef ACQIRIS_DRV_HH
#define ACQIRIS_DRV_HH

#include <dbScan.h>
#include <epicsMutex.h>
#include <epicsEvent.h>

#ifdef __cplusplus
extern "C"
{
#endif

    const int MAX_CHANNEL = 4;
    const int MAX_DEV = 10;
#define SUCCESS(x) (((x)&0x80000000) == 0)

    struct acqiris_data_t
    {
        unsigned nsamples;//actual number of acquired samples per channel
        void *buffer;//acquired data of that channel
    };

    struct acqiris_driver_t
    {
        int module;
        //max. number of channels of that board(no interleaving)
        int nchannels;
        //effective channels when interleaving, see acqiris_drv_lo.cpp
        int effectiveChs;
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
        //see AcqrsD1_readData() in acqiris_daq.cpp
        acqiris_data_t data[MAX_CHANNEL];
        //get more instrument information
        int CrateNb;
        int NbrADCBits;
        int PosInCrate;
        char modelName[32];
        long serialNbr;
        long busNbr;
        long slotNbr;
        double realTrigRate;
        long acqTimeout;
        //the index of the first valid data
        int indexFirstPoint;
        double sampleInterval;
    };
    typedef struct acqiris_driver_t ad_t;

    /*global variable:
     * *array of acqiris driver structure: each element for one card;
     * A mutex is required if multiple boards use DMA to read data
    */
    extern acqiris_driver_t acqiris_drivers[MAX_DEV];
    extern epicsMutexId acqiris_dma_mutex;

#ifdef __cplusplus 
}
#endif

#endif
