/* 
    Original Author: Perazzo, Amedeo
    Modified by Yong Hu: 10/29/2010
*/

#include "acqiris_dev.hh"
#include "acqiris_drv.hh"

#include <dbScan.h>
#include <dbAccess.h>
#include <waveformRecord.h>

//Yong Hu
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <AcqirisD1Import.h>

static double offSet;
static double fullScale;

//July-11-2011: raw Data and voltages are calculated here; others are implemented using asub (acqiris_asub_process.cpp)
//static void rwf_short(void* dst, const void* src, unsigned nsamples)
static void rwf_rawData(void* dst, const void* src, unsigned nsamples)
{
  unsigned size = nsamples*sizeof(short);
  memcpy(dst, src, size);  
}

static void rwf_computedVolt(void* dst, const void* src, unsigned nsamples)
//static void rwf_float(void* dst, void* src, unsigned nsamples)
{
  int i = 0;
  float* fdst = static_cast<float*>(dst);
  const short* ssrc = static_cast<const short*>(src);
 
  for(i=0; i<nsamples; i++)
  {
  	  //fdst[i] = (ssrc[i]+32768.0)/(32704.0+32768.0) + (-0.5);
	  fdst[i] = ((ssrc[i]+32768.0)/(32704.0+32768.0))*fullScale + (-offSet - fullScale/2.0);
  }
}

/* 
 * Add functionalities of data analysis: RMS, Sum, etc.--Yong Hu
 * July-11-2011: raw Data and voltages are calculated here; others are implemented using asub (acqiris_asub_process.cpp)
 */
typedef void (*acqiris_parwf_rfunc)(void* dst, const void* src, unsigned nsamples);
//typedef int (*acqiris_parwf_rfunc)(rec_t* rec, ad_t* ad, float* val);
//typedef int (*acqiris_parao_wfunc)(rec_t* rec, ad_t* ad, double val);

struct acqiris_wfrecord_t
{
  acqiris_parwf_rfunc rfunc;
  //acqiris_parao_wfunc wfunc;
};

//July-11-2011: rwf_*, only rwf_rawData is used/called; others are implemented using asub (acqiris_asub_process.cpp)
#define MaxParwfFuncs 2
static struct _parwf_t {
  const char* name;
  acqiris_parwf_rfunc rfunc;
  //acqiris_parao_wfunc wfunc;
} _parwf[MaxParwfFuncs] = {
  {"WRAW", rwf_rawData},
  {"WVOL", rwf_computedVolt},
};

template<> int acqiris_init_record_specialized(waveformRecord* pwf)
{
//Yong Hu
  acqiris_record_t* arc = reinterpret_cast<acqiris_record_t*>(pwf->dpvt);
  for (unsigned i=0; i<MaxParwfFuncs; i++) 
  {
     if (strcmp(arc->name, _parwf[i].name) == 0) 
     {
      acqiris_wfrecord_t* rwf = new acqiris_wfrecord_t;
      rwf->rfunc = _parwf[i].rfunc;
      arc->pvt = rwf;
      return 0;
     }
   }

  printf("can't initialize waveform record(%s) \n", pwf->name);
  return -1;
 
}

template<> int acqiris_read_record_specialized(waveformRecord* pwf)
{
//Yong Hu
  ViInt32 coupling, bandwidth;
  //double fullScale, offSet;
  ViStatus status;
  
  acqiris_record_t* arc = reinterpret_cast<acqiris_record_t*>(pwf->dpvt);
//arc->module is fetched from INP field (@M${MODULE})
  ad_t* ad = &acqiris_drivers[arc->module];
  acqiris_wfrecord_t* rwf = reinterpret_cast<acqiris_wfrecord_t*>(arc->pvt);
  
  //2011-July-11: for DC222(1-ch, 8GS/s), if 'CHANNEL' in the substitution has 2 instances '0' and '1', will get error 0xBFFC0002
  if( VI_SUCCESS != (status = AcqrsD1_getVertical(ad->id, arc->channel+1, &fullScale,&offSet,&coupling,&bandwidth)))
  {
  	printf("error occurred in the waveform record %s! status: 0x%X; fullScale: %f; offSet: %f \n", pwf->name, status, fullScale, offSet);
  	//return -1;
  }

  const void* buffer = ad->data[arc->channel].buffer;
  //void* buffer = ad->data[arc->channel].buffer;
  unsigned nsamples = ad->data[arc->channel].nsamples;
  //reset NELM equal to nsamples/ch
  pwf->nelm =  nsamples;
  /*
  if (nsamples > pwf->nelm)
  {
    nsamples = pwf->nelm;
	  ad->truncated++;
  }
  */
  //Yong Hu
  //printf("arc_name is:%s, nsamples is: %d\n", arc->name, nsamples);

  epicsMutexLock(ad->daq_mutex);
  //printf("record: %s got daq_mutex \n", arc->name);
  rwf->rfunc(pwf->bptr, buffer, nsamples);
  //memcpy((short *)pwf->bptr, (short *)buffer, nsamples * sizeof(short));//works for raw data copying
  //printf("Volt of the second sample: %f\n", *(buffer+1));
  //printf("record: %s completed waveform memory copy\n", arc->name);
  epicsMutexUnlock(ad->daq_mutex);
  //printf("record: %s released daq_mutex \n", arc->name);
  pwf->nord = nsamples;
  //printf("pwf->nord is: %d\n", nsamples);
  return 0;
}


template<> IOSCANPVT acqiris_getioscanpvt_specialized(waveformRecord* pwf)
{
  acqiris_record_t* arc = reinterpret_cast<acqiris_record_t*>(pwf->dpvt);
  ad_t* ad = &acqiris_drivers[arc->module];
  return ad->ioscanpvt;
}

