/*acqiris_process_asub.cpp: process waveform record: voltage, peak, pulse integral, pulse count;
 * see aSub record "${Psy}-${Ssy}{${Dev}}CVolt-aSub_" in acqiris_channel.template for input/output links
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
#include "dbCommon.h" /* precord: now = paddr->precord->time;*/
#include "epicsTime.h"
#include "waveformRecord.h"

#define MAX_NUM_BUNCH 150
int acqirisAsubDebug = 0;
static bool acqirisAsubInitialized = FALSE;
static bool timeAxisAsubInitialized =  FALSE;
static short *prawData;
static double *pvoltData;
//static double *pbkGroundData;
static double *pfillPattern;
static double *ptimeAxis;
/*global variable*/

typedef long (*processMethod)(aSubRecord *precord);

static long acqirisAsubInit(aSubRecord *precord,processMethod process)
{
	if (acqirisAsubInitialized)
		return (0);
    if (NULL == (prawData = (short *)malloc(precord->noa * sizeof(short)))
    		|| NULL == (pfillPattern = (double *)malloc(precord->noa * sizeof(double)))
    		|| NULL == (pvoltData = (double *)malloc(precord->nova * sizeof(double))))
    {
    	printf("out of memory: acqirisAsubInit \n");
    	return -1;
    }
    else
    {
    	acqirisAsubInitialized = TRUE;
    	//printf("allocate memory in acqirisAsubInit successfully\n");
    }
    return(0);
}

static long acqirisAsubProcess(aSubRecord *precord)
{
	double dcOffset = 0.0;
	double fullScale = 0.0;
	double peakThreshold = 0.0;
	//unsigned short getbkGround = 0;
	unsigned i = 0;
	DBLINK *plink;
	DBADDR *paddr;
	waveformRecord *pwf;
	double max = 0.0;
	double min = 0.0;
	double sum = 0.0;
	double ave = 0.0;
	double std = 0.0;
	//double fillPn[MAX_NUM_BUNCH] = {0.0};
	double b2BMaxVar = 0.0;
	double maxSum = 0.0;
	double minSum = 0.0;
	long numBunch = 0;
	long maxQBunchNum = 0;
	long minQBunchNum = 0;

//input links: raw integer waveform data, input range, offset, background noise waveform, peak search threshold, reset or get bkground
	memcpy(prawData, (short *)precord->a, precord->noa * sizeof(short));
	memcpy(&fullScale, (double *)precord->b, precord->nob * sizeof(double));
	memcpy(&dcOffset, (double *)precord->c, precord->noc * sizeof(double));
	//memcpy(pbkGroundData, (double *)precord->d, precord->nod * sizeof(double));
	memcpy(&peakThreshold, (double *)precord->d, precord->nod * sizeof(double));
	//memcpy(&getbkGround, (unsigned short *)precord->f, precord->nof * sizeof(unsigned short));
	//printf("get all input links values:  peakThreshold %f; getbkGround %d \n", peakThreshold, getbkGround);

//using effective number of samples(samples/channel or 'NELM') in the waveform instead of 'NOA'(max. samples/ch) for data analysis
	plink = &precord->inpa;
	if (DB_LINK != plink->type) return -1;
	//plink->value.pv_link.precord->name;
	//printf("This aSub record name is: %s, %s \n", precord->name, plink->value.pv_link.precord->name);
	paddr = (DBADDR *)plink->value.pv_link.pvt;
	pwf = (waveformRecord *)paddr->precord;
	//printf("number of effective samples(samples/ch): %d \n", pwf->nelm);

//reset background noise: all zero
/*	if (0 == getbkGround)
	{
		memset(pbkGroundData, 0, precord->nod * sizeof(double));
		//printf("reset background noise: all zero \n");
	}
*/
	//printf("threshold for peak searching is %f \n", peakThreshold);

//convert raw data(16-bit integer) to voltages;
	for (i = 0; i < pwf->nelm; i++)
	{
		//pvoltData[i] = fullScale * ((prawData[i]+32768.0)/(32704.0+32768.0)) + (-dcOffset-fullScale/2.0) - pbkGroundData[i];
//Sept.06, 2011: subtracting background noise makes data worse
		pvoltData[i] = fullScale * ((prawData[i]+32768.0)/(32704.0+32768.0)) + (-dcOffset-fullScale/2.0);
	}
//Max, Min, ave, std(RMS noise): sum is not DC component
    max = pvoltData[0];
    min = pvoltData[0];
    //for (i= 0; i < precord->noa; i++)
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
	//printf("RMS noise is: %f \n", std);
 //search positive or negative pulse peaks: number of bunches,Fill pattern, Max variation, individual bunch charge, etc.
    // find peaks and then sum values of 5 points
 //The IOC will consume 100% CPU (2 cores) if using noa=1000000
    //memset(pfillPattern, 0, precord->noa * sizeof(double));
    memset(pfillPattern, 0, pwf->nelm * sizeof(double));
	for (i = 2; i < (pwf->nelm -2); i++)
	{
		if (fabs(pvoltData[i]) > fabs(peakThreshold))
		{
			//printf("#%d data: %f > threshold %f \n", i, pvoltData[i], peakThreshold);
			if ((pvoltData[i-1] < pvoltData[i]) && (pvoltData[i-2] < pvoltData[i]) \
					&& (pvoltData[i] > pvoltData[i+1]) && (pvoltData[i] > pvoltData[i+2]))
			{
				pfillPattern[numBunch] = pvoltData[i-2] + pvoltData[i-1] + pvoltData[i] + pvoltData[i+1] + pvoltData[i+2];
				numBunch++;
			}
		}
	}
/*
	if (numBunch > MAX_NUM_BUNCH)
	{
		printf("number of bunches is %d > %d \n", numBunch, MAX_NUM_BUNCH);
		//return(0);
	}
	printf("number of bunches is %d \n", numBunch);
*/
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

//output links: voltage wf data, max, min, ave, std, NumBunch, FillPn, B2BMaxVar, BunchQ, MaxQBunchNum, MinQBunchNum
	memcpy((double *)precord->vala, pvoltData, precord->nova * sizeof(double));
    memcpy((double *)precord->valb, &max, precord->novb * sizeof(double));
    memcpy((double *)precord->valc, &min, precord->novc * sizeof(double));
    memcpy((double *)precord->vald, &ave, precord->novd * sizeof(double));
    memcpy((double *)precord->vale, &std, precord->nove * sizeof(double));
    memcpy((long *)precord->valf, &numBunch, precord->novf * sizeof(long));
    //memcpy((double *)precord->valg, &pfillPattern[0], precord->novg * sizeof(double));
    memcpy((double *)precord->valg, &pfillPattern[0], 150 * sizeof(double));
    memcpy((double *)precord->valh, &b2BMaxVar, precord->novh * sizeof(double));
  //memcpy((double *)precord->vali, &bunchQ, precord->novi * sizeof(double));
    memcpy((long *)precord->valj, &maxQBunchNum, precord->novj * sizeof(long));
    memcpy((long *)precord->valk, &minQBunchNum, precord->novk * sizeof(long));
	//printf("put all output links values \n");

    if (acqirisAsubDebug)
    {
    	printf("Associated with this asub record %s: \n",precord->name);
    	printf("	the first raw integer data of the waveform: %d; voltage: %fV \n",prawData[0],pvoltData[0]);
    	printf("	DC offset: %fV; Full scale: %fV \n",dcOffset, fullScale);
    	printf("	Max: %fV; Min: %fV \n",max, min);
    }

	return(0);
}


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

    return(0);
}

static long timeAxisAsubProcess(aSubRecord *precord)
{
	unsigned long nSample;
	double sampleLength;
	unsigned int i = 0;
	DBLINK *plink;
	DBADDR *paddr;
	waveformRecord *pwf;

//input links: number of samples(data points), sample length (N ns)
	memcpy(&nSample, (unsigned long *)precord->a, precord->noa * sizeof(unsigned long));
	memcpy(&sampleLength, (double *)precord->b, precord->nob * sizeof(double));

//using effective number of samples(samples/channel or 'NELM') in the waveform instead of 'NOA'(max. samples/ch) for data analysis
	plink = &precord->outa;
	if (DB_LINK != plink->type) return -1;
	//plink->value.pv_link.precord->name;
	//printf("This aSub record name is: %s, %s \n", precord->name, plink->value.pv_link.precord->name);
	paddr = (DBADDR *)plink->value.pv_link.pvt;
	pwf = (waveformRecord *)paddr->precord;
	pwf->nelm = nSample;
	//printf("number of effective samples(samples/ch): %d \n", pwf->nelm);

	for (i = 0; i < pwf->nelm; i++)
	{
		ptimeAxis[i] = i * (sampleLength/nSample);
	}
	//printf("ptimeAxis-1: %f;  ptimeAxis-max: %f;\n", ptimeAxis[1], ptimeAxis[pwf->nelm-1]);

//output links: waveform
	memcpy((double *)precord->vala, ptimeAxis, pwf->nelm * sizeof(double));
	//printf("put all output links values \n");

	return(0);
}

/*RMS noise over 60 samples (1-minute for 1Hz)*/
/*static long ics710ProcessCirBuffer(aSubRecord *precord)
{
    double temp[60];
    int i = 0;
    double sum = 0.0;
    double ave = 0.0;
    double rmsNoise = 0.0;
	//memcpy(&temp, (double *)precord->a, precord->noa * sizeof(precord->fta));
	memcpy(&temp, (double *)precord->a, precord->noa * sizeof(double));
    //printf("noA: %d, size(fta): %d \n", precord->noa, sizeof(precord->fta));
	//memcpy(&temp, (&precord->a)[0], precord->noa * sizeof(double));//This also works
	if (ics710ProcessWfAsubDebug)
	{
		for (i = 0; i < 60; i++) printf("temp[%d] is: %f		",i, temp[i]);
		printf("\n");
	}

	for(i = 0; i < 60; i++)
		sum += temp[i];
	ave = sum / 60.0;
	//printf("sum: %f, ave is %f \n",sum, ave);

	for(i = 0; i < 60; i++)
		rmsNoise += (temp[i] - ave) * (temp[i] - ave);
	rmsNoise = sqrt(rmsNoise / 60.0);
	//printf("rmsNoise is %f \n",rmsNoise);
    *(double *)precord->vala = rmsNoise;
	return(0);
}
*/
/* Register these symbols for use by IOC code: */
//epicsExportAddress(int, ics710AsubDebug);
epicsRegisterFunction(acqirisAsubInit);
epicsRegisterFunction(acqirisAsubProcess);
epicsRegisterFunction(timeAxisAsubInit);
epicsRegisterFunction(timeAxisAsubProcess);
//epicsRegisterFunction(ics710ProcessCirBuffer);

