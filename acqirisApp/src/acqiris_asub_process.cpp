/*acqiris_process_asub.cpp: process waveform record for NSLS-II specific purpose:
 * voltage, peak, pulse integral, pulse count;
 * see aSub record "${PREFIX}VoltWf-ASub_" in acqiris_channel.db for INP/OUT links
 * and "${PREFIX}TimeAxis-ASub_" in acqiris_module.db for associated INP/OUT links
 * */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "registryFunction.h"
#include "aSubRecord.h"
#include "epicsExport.h"
#include "link.h"
#include "dbAddr.h"
#include "dbCommon.h"
#include "waveformRecord.h"

#include "acqiris_drv.hh"
#include "acqiris_dev.hh" //acqiris_record_t; for 'realTrigRate'

const int MAX_NUM_BUNCH = 150;

static bool acqirisAsubInitialized = FALSE;
static bool timeAxisAsubInitialized =  FALSE;
static short *prawData;
static double *pvoltData;
static double *pfillPattern;
static double *ptimeAxis;
/*sampleInterval: calculated in timeAxisAsubProcess(), used in acqirisAsubProcess()
 * for computing absolute charge of each bunch*/
static double sampleInterval;
static unsigned int *pPeakIndex;

typedef long (*processMethod)(aSubRecord *precord);

static long acqirisAsubInit(aSubRecord *precord,processMethod process)
{
	if (acqirisAsubInitialized)
		return (0);
    if (NULL == (prawData = (short *)malloc(precord->noa * sizeof(short))) ||
        NULL == (pfillPattern = (double *)malloc(precord->noa * sizeof(double))) ||
        NULL == (pvoltData = (double *)malloc(precord->nova * sizeof(double))) ||
        NULL == (pPeakIndex = (unsigned int *)malloc(precord->nova * sizeof(unsigned int))))
    {
    	printf("out of memory: acqirisAsubInit \n");
    	return -1;
    }
    else
    {
    	acqirisAsubInitialized = TRUE;
    	//printf("allocate memory in acqirisAsubInit successfully\n");
    }
    return (0);
}

/*This routine is called whenever the DAQ is done: see AcqrsD1_readData() acqiris_daq.cpp
 * ${PREFIX}RawData-Wf_ (I/O Intr): field(FLNK,"${PREFIX}VoltWf-ASub_")*/
static long acqirisAsubProcess(aSubRecord *precord)
{
	double rangeOffset = 0.0; //offset for input range
	double fullScale = 0.0;
	double peakThreshold = 0.0;
	long nbrSampleForSum = 5; //number of samples for integral is odd number
//coefficient for calculation of absolute bunch charge of wall current monitor
	double coefBunchQ = 1.0;
	double zeroingOffset = 0.0; //offset for zeroing noise
	unsigned int i = 0;
	long j = 0; //must be signed
	unsigned int k = 0;
	DBLINK *plink = NULL;
	DBADDR *paddr = NULL;
	waveformRecord *pwf = NULL;
	double max = 0.0;
	double min = 0.0;
	double sum = 0.0;
	double ave = 0.0;
	double std = 0.0;
	double b2BMaxVar = 0.0;
	double maxSum = 0.0;
	double minSum = 0.0;
	long numBunch = 0;
	long maxQBunchNum = 0;
	long minQBunchNum = 0;
	acqiris_record_t *arc = NULL;
	int module = 0;
	acqiris_driver_t *ad = NULL;
	unsigned int nSamples = 0;

/*input links: raw integer waveform data, input range, range offset,
 * peak search threshold,number of samples for integral, etc. */
	memcpy(prawData, (short *)precord->a, precord->noa * sizeof(short));
	memcpy(&fullScale, (double *)precord->b, precord->nob * sizeof(double));
	memcpy(&rangeOffset, (double *)precord->c, precord->noc * sizeof(double));
	memcpy(&peakThreshold, (double *)precord->d, precord->nod * sizeof(double));
	memcpy(&nbrSampleForSum, (long *)precord->e, precord->noe * sizeof(long));
	memcpy(&coefBunchQ, (double *)precord->f, precord->nof * sizeof(double));
	memcpy(&zeroingOffset, (double *)precord->g, precord->nog * sizeof(double));
//below: memcpy(&ad->acqTimeout, (long *)precord->h, precord->noh * sizeof(long));
	//printf("nbrSampleForSum: %d \n", nbrSampleForSum);
	//printf("threshold for peak searching is %f \n", peakThreshold);

/*using effective number of samples(samples/channel or 'NELM') in the waveform
 * instead of 'NOA'(max. samples/ch) for data analysis */
	plink = &precord->inpa;
	if (DB_LINK != plink->type) return -1;
	paddr = (DBADDR *)plink->value.pv_link.pvt;
	pwf = (waveformRecord *)paddr->precord;
/*get MODULE info from INPA: ${PREFIX}RawData-Wf_ and then get 'realTrigRate'
 * which is calculated in acqiris_daq.cpp*/
	arc = (acqiris_record_t *)paddr->precord->dpvt;
	module = arc->module;
	ad = &acqiris_drivers[module];
	memcpy((double *)precord->valn, &ad->realTrigRate, precord->novn * sizeof(double));
//set acquisition timeout (ms): AcqrsD1_waitForEndOfAcquisition(id, timeout);
	memcpy(&ad->acqTimeout, (long *)precord->h, precord->noh * sizeof(long));

//convert raw data(16-bit integer) to voltages;
	//printf("number of effective samples(samples/ch): %d \n", pwf->nelm);
	for (i = 0; i < pwf->nelm; i++)
	{
		pvoltData[i] = fullScale * ((prawData[i]+32768.0)/(32704.0+32768.0))\
				+ (-rangeOffset-fullScale/2.0);

		pvoltData[i] -= zeroingOffset;
	}
/*field (OUTA, "${PREFIX}VoltData-Wf PP"):
 * final voltage waveform data, only copy effective number of samples*/
	memcpy((double *)precord->vala, pvoltData, 	pwf->nelm * sizeof(double));
//dynamically change NELM field of '${PREFIX}VoltData-Wf'(OUTA)
	nSamples = pwf->nelm;//number of effective samples from INPA
	plink = &precord->outa;
	if (DB_LINK != plink->type) return -1;
	paddr = (DBADDR *)plink->value.pv_link.pvt;
	pwf = (waveformRecord *)paddr->precord;//pwf is now associated with OUTA
	pwf->nelm = nSamples;

//Max, Min, ave, std(RMS noise): sum is not DC component
    max = pvoltData[0];
    min = pvoltData[0];
    for (i = 0; i < pwf->nelm; i++)
    {
    	sum += pvoltData[i];
    	if (pvoltData[i] > max)
    		max = pvoltData[i];
    	if (pvoltData[i] < min)
    		min = pvoltData[i];
    }
    ave = sum / pwf->nelm;
    for (i= 0; i < pwf->nelm; i++)
    {
    	std += (pvoltData[i] - ave) * (pvoltData[i] - ave);
    }
    std = sqrt(std/pwf->nelm);
	//printf("RMS noise of %s: %f \n", pwf->name, std);

 /*search positive or negative pulse peaks: number of bunches,normalized fill pattern,
  * Max variation, individual bunch charge (integral), etc.
  * find peaks and then sum values of at least 5 points.
  *in case of negative pulses: convert them to positive to keep the filling pattern algorithm
  * below suitable for either positive or negative beam signal.HOWEVER,this algorithm doesn't
  * work for the signal with both negative and positive peaks, i.e. sinewave with -0.5V~+0.5V
  * */

    //memset consumes lots of CPU load (~100%) if using noa=1000000
    //memset(pfillPattern, 0, precord->noa * sizeof(double));
    memset(pfillPattern, 0, pwf->nelm * sizeof(double));
    for (i=0; i<pwf->nord; i++) // nord == nelm: see acqiris_drv_wf.cpp
    {
    	if (pvoltData[i] < 0.0) pvoltData[i] = (-pvoltData[i]);
    }
//nbrSampleForSum is odd number and >= 5: see ${PREFIX}NbrSamplesForSum-Calc_
	for (i=(nbrSampleForSum-1)/2; i<(pwf->nelm -(nbrSampleForSum-1)/2); i++)
	{
		if (fabs(pvoltData[i]) > fabs(peakThreshold))
		{
			//printf("#%d data: %f > threshold %f \n", i, pvoltData[i], peakThreshold);
//what if there are 2 identical peaks? Does the following solve the problem?
			if ((pvoltData[i-1] < pvoltData[i]) && (pvoltData[i-2] < pvoltData[i]) \
					&& (pvoltData[i] >= pvoltData[i+1]) && (pvoltData[i] >= pvoltData[i+2]))
			{
				for (j=-(nbrSampleForSum-1)/2; j<=(nbrSampleForSum-1)/2; j++)
				{
//integral voltage value of each bunch
					pfillPattern[numBunch] += pvoltData[i+j];
				}
//absolute charge of each bunch
				pfillPattern[numBunch] *= (coefBunchQ * sampleInterval);
				numBunch++;
				pPeakIndex[k++] = i;
			}
		}//if (fabs(pvoltData[i]) > fabs(peakThreshold))
	}//for (i=(nbrSampleForSum-1)/2; i<(pwf->nelm -(nbrSampleForSum-1)/2); i++)

//put integral (absolute bunch charge, 150-bunch for NSLS-2) into OUTL
    memcpy((double *)precord->vall, &pfillPattern[0], MAX_NUM_BUNCH * sizeof(double));

//Normalized filling pattern
    maxSum = pfillPattern[0];
    minSum = pfillPattern[0];
	for (i = 0; i < numBunch; i++)
	{
    	if (pfillPattern[i] > maxSum)
    	{
    		maxSum = pfillPattern[i];
    		maxQBunchNum = i;
    	}
    	if (pfillPattern[i] < minSum)
    	{
    		minSum = pfillPattern[i];
    		minQBunchNum = i;
    	}
	}
	if (maxSum > 0)
	{
		b2BMaxVar = ((maxSum - minSum) / maxSum) * 100.0;
		for (i = 0; i < numBunch; i++)
		{
			pfillPattern[i] /= maxSum;
		}
	}

/*output links: voltage wf data, max, min, ave, std, NumBunch, FillPn,
 *  B2BMaxVar, BunchQ, MaxQBunchNum, MinQBunchNum, PeakIndex, etc. */
//above:memcpy((double *)precord->vala, pvoltData, 	pwf->nelm * sizeof(double));
    memcpy((double *)precord->valb, &max, precord->novb * sizeof(double));
    memcpy((double *)precord->valc, &min, precord->novc * sizeof(double));
    memcpy((double *)precord->vald, &ave, precord->novd * sizeof(double));
    memcpy((double *)precord->vale, &std, precord->nove * sizeof(double));
    memcpy((long *)precord->valf, &numBunch, precord->novf * sizeof(long));
    memcpy((double *)precord->valg, &pfillPattern[0], MAX_NUM_BUNCH * sizeof(double));
    memcpy((double *)precord->valh, &b2BMaxVar, precord->novh * sizeof(double));
//not: memcpy((double *)precord->vali,&BunchQCalib,precord->novi*sizeof(double));
    memcpy((long *)precord->valj, &maxQBunchNum, precord->novj * sizeof(long));
    memcpy((long *)precord->valk, &minQBunchNum, precord->novk * sizeof(long));
//above:memcpy((double *)precord->vall,&pfillPattern[0],MAX_NUM_BUNCH*sizeof(double));
    memcpy((int *)precord->valm, &pPeakIndex[0], MAX_NUM_BUNCH * sizeof(int));
//above:memcpy((double *)precord->valn,&ad->realTrigRate,precord->novn*sizeof(double));

	//printf("put all output links values of %s in acqirisAsubProcess()\n",precord->name);
	return (0);
}


/*see "${PREFIX}TimeAxis-ASub_" in acqiris_module.db for associated INP/OUT links*/
static long timeAxisAsubInit(aSubRecord *precord,processMethod process)
{
	if (timeAxisAsubInitialized)
		return (0);
    if (NULL == (ptimeAxis = (double *)malloc(precord->nova * sizeof(double))))
    {
    	printf("out of memory: timeAxisAsubInit \n");
    	return -1;
    }
    else
    {
    	timeAxisAsubInitialized = TRUE;
    	//printf("allocate memory in timeAxisAsubInit successfully\n");
    }

    return (0);
}

/*This routine is called only when nSamples or sampleLength is changed*/
static long timeAxisAsubProcess(aSubRecord *precord)
{
	long nSample;
	double sampleLength;
	long i = 0;
	DBLINK *plink;
	DBADDR *paddr;
	waveformRecord *pwf;

//input links: number of samples(data points), sample length (N ns)
	memcpy(&nSample, (long *)precord->a, precord->noa * sizeof(long));
	memcpy(&sampleLength, (double *)precord->b, precord->nob * sizeof(double));
//sampleInterval is used for beam charge calculation in acqirisAsubProcess()
	sampleInterval = sampleLength / nSample;
/*get effective number of samples(samples/channel or 'NELM') in the waveform
 * instead of 'NOA'(max. samples/ch) for data analysis*/
	plink = &precord->outa;
	if (DB_LINK != plink->type) return -1;
	paddr = (DBADDR *)plink->value.pv_link.pvt;
	pwf = (waveformRecord *)paddr->precord;
	pwf->nelm = nSample;
    //printf("number of effective samples(samples/ch): %d \n", pwf->nelm)
	for (i = 0; i < pwf->nelm; i++)
	{
		ptimeAxis[i] = i * (sampleLength/nSample);
	}
	memcpy((double *)precord->vala, ptimeAxis, pwf->nelm * sizeof(double));

	//printf("put all output links values of %s in timeAxisAsubProcess()\n",precord->name);
	return (0);
}

epicsRegisterFunction(acqirisAsubInit);
epicsRegisterFunction(acqirisAsubProcess);
epicsRegisterFunction(timeAxisAsubInit);
epicsRegisterFunction(timeAxisAsubProcess);
