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

#define RMS_NUM 16
static double offSet; 

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
  	fdst[i] = (ssrc[i]+32768.0)/(32704.0+32768.0) + (-(float)offSet-0.5); 
  }
}

//static void rwf_double(void* dst, const void* src, unsigned nsamples)
static void rwf_RMSVolt(void* dst, const void* src, unsigned nsamples)
{    
  int i = 0;
  int j = 0;
  float fsrc[5000];
 
  float* fdst = static_cast<float*>(dst);
  const short* ssrc = static_cast<const short*>(src);
  
  while(i < nsamples)
  {
  	fdst[i] = 0;
  	while(j < nsamples*RMS_NUM)
  	{
  		fsrc[j] = (ssrc[j]+32768.0)/(32704.0+32768.0) + (-(float)offSet-0.5);
  		fsrc[j] = fsrc[j] * fsrc[j];
  		if(0 != (j+1) % RMS_NUM)
  		{
  			fdst[i] += fsrc[j];
  			j++;
  		}
  		else
  		{
  			fdst[i] += fsrc[j];
  			fdst[i] /= RMS_NUM;
  			fdst[i] = sqrt(fdst[i]);
  			j++;
  			i++;
  			break;
  		}
  	} 
  }
  
}

static void rwf_MaxVolt(void* dst, const void* src, unsigned nsamples)
{    
  int i = 0;
  int j = 0;
  float max = 0.0;
  float fsrc[5000];
  float* fdst = static_cast<float*>(dst);
  const short* ssrc = static_cast<const short*>(src);
  
  fsrc[0] = (ssrc[0]+32768.0)/(32704.0+32768.0) + (-(float)offSet-0.5);
  max = fsrc[0];
  
  //while(i < nsamples)
  while((i < nsamples) && (j < nsamples*RMS_NUM))
  {
  	//printf("i is: %d; j is: %d; nsamples is: %d\n", i, j, nsamples);
  	fsrc[j] = (ssrc[j]+32768.0)/(32704.0+32768.0) + (-(float)offSet-0.5);
  	//while(j < nsamples*RMS_NUM)
	if(0 != (j+1) % RMS_NUM)
	{
        if(max < fsrc[j]) max = fsrc[j];
		j++;
	}
	else
	{
		if(max < fsrc[j]) max = fsrc[j];
	    j++;
		fdst[i] = max;
		i++;
		//break;
	}
  
  }
  
}


/* 
 * Add functionalities of data analysis: RMS, Sum, etc.--Yong Hu
 */
typedef void (*acqiris_parwf_rfunc)(void* dst, const void* src, unsigned nsamples);
//typedef int (*acqiris_parwf_rfunc)(rec_t* rec, ad_t* ad, float* val);
//typedef int (*acqiris_parao_wfunc)(rec_t* rec, ad_t* ad, double val);

struct acqiris_wfrecord_t
{
  acqiris_parwf_rfunc rfunc;
  //acqiris_parao_wfunc wfunc;
};

#define MaxParwfFuncs 4
static struct _parwf_t {
  const char* name;
  acqiris_parwf_rfunc rfunc;
  //acqiris_parao_wfunc wfunc;
} _parwf[MaxParwfFuncs] = {
  {"WRAW", rwf_rawData},
  {"WVOL", rwf_computedVolt},
  {"WRMS", rwf_RMSVolt},
  {"WMAX", rwf_MaxVolt},
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
  return -1;
 
}

template<> int acqiris_read_record_specialized(waveformRecord* pwf)
{
//Yong Hu
  ViInt32 coupling, bandwidth;
  double fullScale;
  ViStatus status;
  
  acqiris_record_t* arc = reinterpret_cast<acqiris_record_t*>(pwf->dpvt);
//arc->module is fetched from INP field (@M${MODULE})
  ad_t* ad = &acqiris_drivers[arc->module];
  acqiris_wfrecord_t* rwf = reinterpret_cast<acqiris_wfrecord_t*>(arc->pvt);
  
  //Yong Hu 
  if( 0 != AcqrsD1_getVertical(ad->id, arc->channel+1,&fullScale,&offSet,&coupling,&bandwidth))
  {
  	printf("Can't get vertical offSet value --YHU \n");
  	return -1;
  }
                          
  const void* buffer = ad->data[arc->channel].buffer;
  //void* buffer = ad->data[arc->channel].buffer;
  unsigned nsamples = ad->data[arc->channel].nsamples;
  if (nsamples > pwf->nelm) {
    nsamples = pwf->nelm;
    ad->truncated++;
  }
  //Yong Hu
  //printf("arc_name is:%s, nsamples is: %d\n", arc->name, nsamples);
  if ((strcmp(arc->name, "WRMS") == 0) || (strcmp(arc->name, "WMAX") == 0)) 
  //if (strcmp(arc->name, "WRMS") == 0) 
  {
  	nsamples /= RMS_NUM;
  	//printf("arc_name is:%s, nsamples is: %d\n", arc->name, nsamples);
  }
  
  epicsMutexLock(ad->daq_mutex);
  //printf("record: %s got daq_mutex \n", arc->name);
  rwf->rfunc(pwf->bptr, buffer, nsamples);
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

