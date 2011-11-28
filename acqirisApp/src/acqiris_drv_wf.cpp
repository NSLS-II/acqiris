/* 
    Original Author: Perazzo, Amedeo
    Current Author: Hu, Yong <yhu@bnl.gov>
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>

#include "dbScan.h"
#include "waveformRecord.h"
#include "epicsPrint.h"

#include "acqiris_drv.hh"
#include "acqiris_dev.hh"

template<> int acqiris_init_record_specialized(waveformRecord* pwf)
{
	int ret = 0;

	acqiris_record_t* arc = reinterpret_cast<acqiris_record_t*>(pwf->dpvt);
	if (0 != strcmp(arc->name, "WRAW"))
	{
		printf("@parm should be 'WRAW' in the record(%s) \n", pwf->name);
		ret = -1;
	}

	return (ret);
}

template<> int acqiris_read_record_specialized(waveformRecord* pwf)
{
	acqiris_record_t* arc = reinterpret_cast<acqiris_record_t*>(pwf->dpvt);
//arc->module is fetched from INP field (@M${MODULE})
	ad_t* ad = &acqiris_drivers[arc->module];
	unsigned nsamples = ad->data[arc->channel].nsamples;

//bug fix: the followings are required if combining channels and initial nsamples is 0
	if (0 == nsamples)
	{
		nsamples = 1;
		pwf->nord = nsamples;
		//printf("pwf->nord is: %d\n", nsamples);
		return 0;
	}

	const void* buffer = ad->data[arc->channel].buffer;
	//printf("record name is: %s, arc_name is:%s, nsamples is: %d\n",\
			pwf->name, arc->name, nsamples);
	try
	{
//make sure the memory size(N bytes) of pwf->bptr & buffer >= nsamples * sizeof(short)
		memcpy(pwf->bptr, buffer, nsamples * sizeof(short));
		//printf("Volt of the second sample: %f\n", *(buffer+1));
		//printf("record: %s completed waveform memory copy\n", arc->name);
	}
	catch (std::exception &e)
	{
		errlogPrintf("Error occurred during buffer copy: %s\n",e.what());
	}

/*reset NELM to the actual number of samples obtained by AcqrsD1_readData()
 * in acqiris_daq.cpp so that CSS/EDM can dynamically display waveform data correctly
 * */
	pwf->nelm = nsamples;
//waveform readout is not complete until pwf->nord is set
	pwf->nord = nsamples;
	//printf("pwf->nord of the wf_record %s is: %d\n", pwf->name, nsamples);

	return 0;
}

//get_ioint_info is only called during iocInit: returns ioscanpvt
template<> IOSCANPVT acqiris_getioscanpvt_specialized(waveformRecord* pwf)
{
	acqiris_record_t* arc = reinterpret_cast<acqiris_record_t*>(pwf->dpvt);
	ad_t* ad = &acqiris_drivers[arc->module];
	printf("ioscanpvt returned from the wf_record %s \n", pwf->name);
	return ad->ioscanpvt;
}

