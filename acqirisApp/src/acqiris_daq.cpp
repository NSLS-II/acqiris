/* 
    Original Author: Perazzo, Amedeo
    Modified by Yong Hu: 10/29/2010
*/

#include "acqiris_daq.hh"
#include "acqiris_drv.hh"

#include <AcqirisD1Import.h>
#include <AcqirisImport.h>

#include <stdio.h>

/*Yong Hu*/
#include <epicsTime.h>

#define SUCCESSREAD(x) (((x)&0x80000000)==VI_SUCCESS)

extern "C"
{
  void acqiris_daq_thread(void *arg)
  {
//Yong Hu
    epicsTimeStamp now;
    char tsText[30];

    acqiris_driver_t* ad = reinterpret_cast<acqiris_driver_t*>(arg);
    int nchannels = ad->nchannels;
    int extra = ad->extra;

    const int nbrSegments = 1;
    int nbrSamples = ad->maxsamples;

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
//Yong Hu
     //printf("ad->running = %d --yhu\n", ad->running);
     //Acqrs_calibrateCancel(ad->id);
     //printf("Cancel the calibration process after initialization--yhu\n");

    while (1) {
      epicsEventWait(ad->run_semaphore);
      do { 
//Yong Hu: use shorter timeout for testing 120Hz: 10ms is too short, 20ms works well;
        const long timeout = 1000; /* ms */
        //const long timeout = 10; /* ms */
        //const long timeout = 20; /* ms */
        int id = ad->id;
        //AcqrsD1_acquire(id); 
//Yong Hu
        ViStatus status = AcqrsD1_acquire(id); 
        if (status != VI_SUCCESS) {
          printf("Can't start the acquisition --yhu\n");
        }

        status = AcqrsD1_waitForEndOfAcquisition(id, timeout);
        if (status != VI_SUCCESS) {
//Yong Hu
          printf("Timeout when waitForEndofAcquisition: status code is 0x%x, the time is: ", (unsigned)status);
          epicsTimeGetCurrent(&now);
          epicsTimeToStrftime(tsText, sizeof(tsText), "%Y-%m-%d %H:%M:%S.%6f", &now);
          printf("%s \n", tsText);
          //epicsTimeShow(&now, 0);

          AcqrsD1_stopAcquisition(id);
          ad->timeouts++;
        } else {
          epicsMutexLock(ad->daq_mutex);
          for (int channel=0; channel<nchannels; channel++) {
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
            } else {
              ad->data[channel].nsamples = 0;
              ad->readerrors++;
            }
          }
          epicsMutexUnlock(ad->daq_mutex);
          scanIoRequest(ad->ioscanpvt);
          ad->count++;
        }
      } while (ad->running);
    }
  }
}
