/* 
    Original Author: Perazzo, Amedeo
    Modified/modified by Yong Hu: 09/29/2011
*/

#include "acqiris_daq.hh"
#include "acqiris_drv.hh"

#include <AcqirisD1Import.h>
#include <AcqirisImport.h>

#include <stdio.h>

/*Yong Hu*/
#include <epicsTime.h>

#define SUCCESSREAD(x) (((x)&0x80000000)==VI_SUCCESS)

/*global variable*/
//double iocUpdateRate;

extern "C"
{
  long timeout = 2000;//acquisition timeout declared in acqiris_drv.hh

  void acqiris_daq_thread(void *arg)
  {
//Yong Hu
    epicsTimeStamp now;
    char tsText[30];
    //short *buffer;
    double preTimeAtEndOfAcq = 0.0;
    double curTimeAtEndOfAcq = 0.0;

    acqiris_driver_t* ad = reinterpret_cast<acqiris_driver_t*>(arg);
    int nchannels = ad->nchannels;//max. number of channels (no combination/interleaving)
    int extra = ad->extra;//extra=208
    //int extra = 32;
    const int nbrSegments = 1;
    int nbrSamples = ad->maxsamples;//512K for stardard DC252
    //int nbrSamples = 4000;
    //printf("extra: %d;nbrSamples: %d \n", ad->extra, ad->maxsamples );

    AqReadParameters    readParams;
    AqDataDescriptor    wfDesc;
    AqSegmentDescriptor segDesc[nbrSegments];

    readParams.dataType         = ReadInt16;
    readParams.readMode         = ReadModeStdW;
    readParams.nbrSegments      = nbrSegments;
    readParams.firstSampleInSeg = 0;
    readParams.firstSegment     = 0;
    readParams.segmentOffset    = 0;
    readParams.segDescArraySize = nbrSegments*sizeof(AqSegmentDescriptor);
    readParams.nbrSamplesInSeg  = nbrSamples;
    readParams.dataArraySize    = (nbrSamples+extra)*nbrSegments*sizeof(short);
    //yhu: bug fix: AqReadDataFlags is default to 5; must set readParams.flags=0 to display waveform data stably/correctly
    readParams.flags = 0;
    readParams.reserved = 0;
    readParams.reserved2 = 0.0;
    readParams.reserved3 = 0.0;
//Yong Hu
     //printf("ad->running = %d --yhu\n", ad->running);
     //Acqrs_calibrateCancel(ad->id);
     //printf("Cancel the calibration process after initialization--yhu\n");

    while (1) {
      epicsEventWait(ad->run_semaphore);
      do { 
//Yong Hu: use shorter timeout for testing 120Hz: 10ms is too short, 20ms works well;
        //const long timeout = 2000; /* ms */
        //const long timeout = 10; /* ms */
        //const long timeout = 20; /* ms */
        int id = ad->id;
        //AcqrsD1_acquire(id); 
//Yong Hu
        ViStatus status = AcqrsD1_acquire(id); 
        if (status != VI_SUCCESS) {
          printf("Can't start the acquisition --yhu\n");
        }

        //printf("Acquisition timout is set to: %d ms \n",timeout);
        status = AcqrsD1_waitForEndOfAcquisition(id, timeout);
//compute trigger rate (ioc update rate): should be calculated here or below (if ... else)?
        epicsTimeGetCurrent(&now);
        curTimeAtEndOfAcq = now.secPastEpoch + now.nsec/1000000000.0;
        ad->realTrigRate = 1.0/(curTimeAtEndOfAcq - preTimeAtEndOfAcq);
        preTimeAtEndOfAcq = curTimeAtEndOfAcq;

        if (status != VI_SUCCESS) {
//Yong Hu
          printf("Timeout occurred on card #%d when waitForEndofAcquisition: status code is 0x%x, the time is: ", ad->module,(unsigned)status);
          epicsTimeGetCurrent(&now);
          epicsTimeToStrftime(tsText, sizeof(tsText), "%Y-%m-%d %H:%M:%S.%6f", &now);
          printf("%s \n", tsText);
          //epicsTimeShow(&now, 0);
          AcqrsD1_stopAcquisition(id);
          ad->timeouts++;
        } else {
          epicsMutexLock(ad->daq_mutex);
          //yhu: bug fix: nchannels is the max. number of channels. Should use actual effective channels,
          //otherwise, AcqrsD1_readData will return without success
          //for (int channel=0; channel<nchannels; channel++) {
          //ad->effectiveChs is set in acqiris_drv_lo.cpp
          for (int channel=0; channel < ad->effectiveChs; channel++) {
            void* buffer = ad->data[channel].buffer;
            epicsMutexLock(acqiris_dma_mutex);
            status = AcqrsD1_readData(id, 
                                      channel+1, 
                                      &readParams, 
                                      buffer,
                                      &wfDesc, 
                                      &segDesc);
            epicsMutexUnlock(acqiris_dma_mutex);
            if (SUCCESSREAD(status)) {
              ad->data[channel].nsamples = wfDesc.returnedSamplesPerSeg;
              //yhu: bug fix: DataArray[indexFirstPoint]... DataArray[indexFirstPoint+ returnedSamplesPerSeg-1]
              ad->data[channel].buffer = (void *)((short *)buffer + wfDesc.indexFirstPoint);
              //printf("indexFirstPoint(the first valid point): %d; number of actual acquired samples: %d \n", wfDesc.indexFirstPoint, wfDesc.returnedSamplesPerSeg);
              //printf("Horizontal position of first data point: %f;  readParams.flags: %d \n", segDesc[nbrSegments].horPos, readParams.flags);
            } else {
              ad->data[channel].nsamples = 0;
              printf(" AcqrsD1_readData() error on card #%d, error status: 0x%X \n", ad->module, status);
              ad->readerrors++;
            }
          }//for (int channel=0; channel<nchannels; channel++)
          epicsMutexUnlock(ad->daq_mutex);
          scanIoRequest(ad->ioscanpvt);
          ad->count++;
        }
      } while (ad->running);
    }
  }
}
