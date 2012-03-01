/* 
 Original Author: Perazzo, Amedeo
 Current Author: Hu, Yong <yhu@bnl.gov>
 */

#include <stdio.h>

#include "AcqirisD1Import.h"
#include "AcqirisImport.h"

#include "epicsTime.h"
#include "epicsMutex.h"
#include "epicsPrint.h"

#include "acqiris_daq.hh"
#include "acqiris_drv.hh"

#define SUCCESSREAD(x) (((x)&0x80000000)==VI_SUCCESS)

void
acqiris_daq_thread(void *arg)
{
    epicsTimeStamp now;
    char tsText[30];
    double preTimeAtEndOfAcq = 0.0;
    double curTimeAtEndOfAcq = 0.0;
    const int nbrSegments = 1;
    AqReadParameters readParams;
    AqDataDescriptor wfDesc;
    AqSegmentDescriptor segDesc[nbrSegments];

    acqiris_driver_t* ad = reinterpret_cast<acqiris_driver_t*> (arg);

    ad->acqTimeout = 2000;
    long timeout = ad->acqTimeout;

    int extra = ad->extra;
    int nbrSamples = ad->maxsamples;
    //printf("extra: %d;nbrSamples: %d \n", ad->extra, ad->maxsamples );
    /** bug fix: using the following 'void * buffer' will produce identical
     * data for all channels, i.e. DC252: Ch1 has the data which is from Ch2
     */
    //int size = (ad->maxsamples + ad->extra) * sizeof(short);
    //void* buffer = new char[size];

    epicsMutexId acqiris_dma_mutex = epicsMutexMustCreate();

    //for 8-bit digitizer, dataType is 0. ReadInt16 for 10- or 12-bit
    if (8 == ad->NbrADCBits)
    {
        readParams.dataType = ReadInt8;
    }
    else if ((ad->NbrADCBits > 8) && (ad->NbrADCBits < 16))
    {
        readParams.dataType = ReadInt16;
    }
    else
    {
        errlogPrintf("unknown ADC resolution: not 8-, or 10- or 12-bit \n");
    }
    readParams.readMode = ReadModeStdW;
    readParams.nbrSegments = nbrSegments;
    readParams.firstSampleInSeg = 0;
    readParams.firstSegment = 0;
    readParams.segmentOffset = 0;
    readParams.segDescArraySize = nbrSegments * sizeof(AqSegmentDescriptor);
    readParams.nbrSamplesInSeg = nbrSamples;
    readParams.dataArraySize = (nbrSamples + extra) * nbrSegments
            * sizeof(short);
    /**bug fix: AqReadDataFlags is default to 5;
     must set readParams.flags=0 to display waveform data stably/correctly
     */
    readParams.flags = 0;
    readParams.reserved = 0;
    readParams.reserved2 = 0.0;
    readParams.reserved3 = 0.0;

    while (1)
    {
        epicsEventWait(ad->run_semaphore);

        do
        {
            int id = ad->id;
            ViStatus status = AcqrsD1_acquire(id);
            if (VI_SUCCESS != status)
            {
                errlogPrintf("card#%d can't start the acquisition\n",
                        ad->module);
            }

            //printf("card#%d AcqTimeout is:%d ms\n",ad->module,timeout);
            status = AcqrsD1_waitForEndOfAcquisition(id, timeout);

            //compute ioc update rate: should be here or below (if ... else)?
            epicsTimeGetCurrent(&now);
            curTimeAtEndOfAcq = now.secPastEpoch + now.nsec / 1000000000.0;
            ad->realTrigRate = 1.0 / (curTimeAtEndOfAcq - preTimeAtEndOfAcq);
            preTimeAtEndOfAcq = curTimeAtEndOfAcq;

            if (VI_SUCCESS != status)
            {
                errlogPrintf(
                        "Timeout on card #%d when waitForEndofAcquisition:\n",
                        ad->module);
                errlogPrintf("\t status code is 0x%x, the time is ",
                        (unsigned) status);
                epicsTimeGetCurrent(&now);
                epicsTimeToStrftime(tsText, sizeof(tsText),
                        "%Y-%m-%d %H:%M:%S.%6f", &now);
                errlogPrintf("%s \n", tsText);
                //epicsTimeShow(&now, 0);
                AcqrsD1_stopAcquisition(id);
                ad->timeouts++;
            }
            else
            {
                /** bug fix: use effective channels instead of the max. number
                 * of channels.Or AcqrsD1_readData will return without success
                 */
                //ad->effectiveChs is set in acqiris_drv_lo.cpp
                //epicsMutexLock(ad->daq_mutex);
                for (int channel = 0; channel < ad->effectiveChs; channel++)
                {
                    void *buffer = ad->data[channel].buffer;

                    epicsMutexLock(acqiris_dma_mutex);
                    status = AcqrsD1_readData(id, channel + 1, &readParams,
                            buffer, &wfDesc, &segDesc);
                    epicsMutexUnlock(acqiris_dma_mutex);

                    if (SUCCESSREAD(status))
                    {
                        /*returned samples per Seg is configured by
                         AcqrsD1_configMemory() in acqiris_drv_lo.cpp
                         */
                        ad->data[channel].nsamples
                                = wfDesc.returnedSamplesPerSeg;
                        /*bugs fix:
                         * 1. fix "waveform display jitter:
                         *		use DataArray[indexFirstPoint,
                         *		indexFirstPoint+returnedSamplesPerSeg-1];
                         * 2. fix segmentation fault:
                         * 		use 'void* buffer = new char[size]' instead of
                         *          'void *buffer = ad->data[channel].buffer'
                         *    to prevent these pointers being modified;
                         * 3. 02/06/2012:don't use 'void* buffer = new char[size]'
                         *    and put 'wfDesc.indexFirstPoint' into acqiris_drv_wf.cpp
                         * */
                        //ad->data[channel].buffer = (void *) ((short *) buffer
                        //        + wfDesc.indexFirstPoint);
                        ad->indexFirstPoint = wfDesc.indexFirstPoint;
                        /*printf("card#%d: indexFirstPoint: %d;
                         * returnedSamplesPerSeg: %d \n",ad->module,
                         * wfDesc.indexFirstPoint,wfDesc.returnedSamplesPerSeg);
                         * */
                    }
                    else
                    {
                        ad->data[channel].nsamples = 0;
                        errlogPrintf(
                                " AcqrsD1_readData() error on card #%d, status: 0x%X \n",
                                ad->module, status);
                        ad->readerrors++;
                    }
                }//for (int channel=0; channel<nchannels; channel++)
                //epicsMutexUnlock(ad->daq_mutex);

                ad->count++;

                scanIoRequest(ad->ioscanpvt);
                //printf("card#%d send out iocsanpvt \n", ad->module);
            }//if (status != VI_SUCCESS) ... else
        }//do
        while (ad->running);
    }//while(1)
}

