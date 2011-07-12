/*acqiris_process_asub.cpp: process waveform record: voltage, peak, pulse integral, pulse count; */

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

int acqirisAsubDebug = 0;
static bool acqirisAsubInitialized = FALSE;
static short *prawData;
static double *pvoltData;
/*global variable*/

typedef long (*processMethod)(aSubRecord *precord);

static long acqirisAsubInit(aSubRecord *precord,processMethod process)
{
	if (acqirisAsubInitialized)
		return (0);
    if (NULL == (prawData = (short *)malloc(precord->noa * sizeof(short)))
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
	unsigned i = 0;
	DBLINK *plink;
	DBADDR *paddr;
	waveformRecord *pwf;
	double max;
	double min;

	memcpy(prawData, (short *)precord->a, precord->noa * sizeof(short));
	memcpy(&fullScale, (double *)precord->b, precord->nob * sizeof(double));
	memcpy(&dcOffset, (double *)precord->c, precord->noc * sizeof(double));

//convert raw data(16-bit integer) to voltages
    for(i = 0; i < precord->noa; i++)
    {
    	pvoltData[i] = fullScale * ((prawData[i]+32768.0)/(32704.0+32768.0)) + (-dcOffset-fullScale/2.0);
    }
	memcpy((double *)precord->vala, pvoltData, precord->nova * sizeof(double));

//using effective number of samples(samples/channel or 'NELM') in the waveform instead of 'NOA' for data analysis
	plink = &precord->inpa;
	if (DB_LINK != plink->type) return -1;
	//plink->value.pv_link.precord->name;
	//printf("This aSub record name is: %s, %s \n", precord->name, plink->value.pv_link.precord->name);
	paddr = (DBADDR *)plink->value.pv_link.pvt;
	pwf = (waveformRecord *)paddr->precord;
	//printf("number of effective samples(samples/ch): %d \n", pwf->nelm);

//Max. & Min values
    max = pvoltData[0];
    min = pvoltData[0];
    //for (i= 0; i < precord->noa; i++)
    for (i= 0; i < pwf->nelm; i++)
    {
    	if (pvoltData[i] > max)
    		max = pvoltData[i];
    	if (pvoltData[i] < min)
    		min = pvoltData[i];
    }
    memcpy((double *)precord->valb, &max, precord->novb * sizeof(double));
    memcpy((double *)precord->valc, &min, precord->novc * sizeof(double));

    // locate peak values (positive and negative)

    if (acqirisAsubDebug)
    {
    	printf("Associated with this asub record %s: \n",precord->name);
    	printf("	the first raw integer data of the waveform: %d; voltage: %fV \n",prawData[0],pvoltData[0]);
    	printf("	DC offset: %fV; Full scale: %fV \n",dcOffset, fullScale);
    	printf("	Max: %fV; Min: %fV \n",max, min);
    }

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
//epicsRegisterFunction(ics710ProcessCirBuffer);

