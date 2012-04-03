/* process waveform record for NSLS-II specific purpose:
 * voltage, peak, pulse integral, pulse count;
 * see record "${PREFIX}VoltWf-ASub_" in acqiris_channel.db for INP/OUT links
 * and "${PREFIX}TimeAxis-ASub_" in acqiris_module.db for INP/OUT links
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
#include "longoutRecord.h"
#include "errlog.h"

#include "acqiris_drv.hh"
#include "acqiris_dev.hh" //acqiris_record_t; for 'realTrigRate'
const int MAX_NUM_BUNCH = 150;

static bool acqirisAsubInitialized = FALSE;
static bool timeAxisAsubInitialized = FALSE;
static short *prawData;
static double *pvoltData;
static double *pfillPattern;
static double *ptimeAxis;
static char *pcharData = NULL;
static unsigned int *pPeakIndex;

typedef long
(*processMethod)(aSubRecord *precord);

static long
acqirisAsubInit(aSubRecord *precord, processMethod process)
{
    if (acqirisAsubInitialized)
        return (0);
    if (NULL == (pcharData = (char *) malloc(precord->noa * sizeof(short)))
            || NULL == (prawData = (short *) malloc(
                    precord->noa * sizeof(short))) || NULL == (pfillPattern
            = (double *) malloc(precord->noa * sizeof(double))) || NULL
            == (pvoltData = (double *) malloc(precord->nova * sizeof(double)))
            || NULL == (pPeakIndex = (unsigned int *) malloc(
                    precord->nova * sizeof(unsigned int))))
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

//basic processing: max, min, sum/integral, mean, std; called by processWf()
void
processBasic(const double *pData, unsigned startPoint, unsigned endPoint,
             double *max, double *min, double *sum, double *ave, double *std)
{
    //*max = pData[0];
    //*min = pData[0];
    *max = pData[startPoint];
    *min = pData[startPoint];
    *sum = 0.0;
    *ave = 0.0;
    *std = 0.0;
    unsigned int i = 0;

    //base data processing: max, min, sum/integral, mean, std
    for (i = startPoint; i < endPoint; i++)
    {
        if (pData[i] > *max)
            *max = pData[i];
        if (pData[i] < *min)
            *min = pData[i];
        *sum += pData[i];
    }
    *ave = (*sum) / (endPoint - startPoint);
    //calculate standard deviation (RMS noise: Delta = sqrt[(Xi-Xmean)^2)]
    for (i = startPoint; i < endPoint; i++)
    {
        *std += (pData[i] - (*ave)) * (pData[i] - (*ave));
    }
    *std = sqrt((*std) / (endPoint - startPoint));

    return;
}

/*This routine is called whenever the DAQ is done: see AcqrsD1_readData()
 * acqiris_daq.cpp ${MON}Raw-Wf (I/O Intr):
 * field(FLNK,"${MON}Raw-aSub_")
 * */
static long
acqirisAsubProcess(aSubRecord *precord)
{
    double rangeOffset = 0.0; //offset for input range
    double fullScale = 0.0;
    double peakThreshold = 0.0;
    long nbrSampleForSum = 5; //number of samples for integral is odd number
    //coefficient for calculation of bunch charge of wall current monitor
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
    double maxROI = 0.0;
    double minROI = 0.0;
    double sumROI = 0.0;
    double aveROI = 0.0;
    double stdROI = 0.0;
    double b2BMaxVar = 0.0;
    double maxSum = 0.0;
    double minSum = 0.0;
    double qSum = 0.0;
    long numBunch = 0;
    long maxQBunchNum = 0;
    long minQBunchNum = 0;
    acqiris_record_t *arc = NULL;
    int module = 0;
    acqiris_driver_t *ad = NULL;
    unsigned int nSamples = 0;
    //char *pcharData = NULL;

    /*input links: raw integer waveform data, input range, range offset,
     * peak search threshold,number of samples for integral, etc.
     * */
    //get MODULE info from INPA: ${MON}Raw-Wf
    plink = &precord->inpa;
    if (DB_LINK != plink->type)
        return -1;
    paddr = (DBADDR *) plink->value.pv_link.pvt;
    pwf = (waveformRecord *) paddr->precord;
    arc = (acqiris_record_t *) paddr->precord->dpvt;
    module = arc->module;
    ad = &acqiris_drivers[module];

    if (8 == ad->NbrADCBits)
    {
        //precord->fta = 1;
        //pcharData = (char *) precord->a;
        memcpy(pcharData, precord->a, precord->noa * sizeof(short));
    }
    else
    {
        memcpy(prawData, (short *) precord->a, precord->noa * sizeof(short));
    }
    memcpy(&fullScale, (double *) precord->b, precord->nob * sizeof(double));
    memcpy(&rangeOffset, (double *) precord->c, precord->noc * sizeof(double));
    memcpy(&peakThreshold, (double *) precord->d, precord->nod * sizeof(double));
    memcpy(&nbrSampleForSum, (long *) precord->e, precord->noe * sizeof(long));
    memcpy(&coefBunchQ, (double *) precord->f, precord->nof * sizeof(double));
    memcpy(&zeroingOffset, (double *) precord->g, precord->nog * sizeof(double));
    memcpy(&ad->acqTimeout, (long *) precord->h, precord->noh * sizeof(long));
    //printf("nbrSampleForSum: %d \n", nbrSampleForSum);
    //printf("threshold for peak searching is %f \n", peakThreshold);

    /*using effective number of samples ('NELM') in the waveform
     * instead of 'NOA'(max. samples/ch) for data analysis
     * */
    //printf("number of effective samples(samples/ch): %d \n", pwf->nelm);
    //convert raw data(8-bit or 16-bit) to voltages: [-off-fs/2, -off+fs/2]
    if (10 == ad->NbrADCBits)
    {
        for (i = 0; i < pwf->nelm; i++)
        {
            //10-bit raw ADC data as 16-bit in the range [-32768, 32704]
            pvoltData[i] = fullScale * ((prawData[i] + 32768.0) / (32704.0
                    + 32768.0)) + (-rangeOffset - fullScale / 2.0);

            pvoltData[i] -= zeroingOffset;
        }
    }
    else if (12 == ad->NbrADCBits)
    {
        for (i = 0; i < pwf->nelm; i++)
        {
            //12-bit raw ADC data as 16-bit in the range [-32768, 32752]
            pvoltData[i] = fullScale * ((prawData[i] + 32768.0) / (32752.0
                    + 32768.0)) + (-rangeOffset - fullScale / 2.0);

            pvoltData[i] -= zeroingOffset;
        }
    }
    else //8-bit: [-128,+127]
    {
        for (i = 0; i < pwf->nelm; i++)
        {
            pvoltData[i] = fullScale * ((pcharData[i] + 128.0)
                    / (127.0 + 128.0)) + (-rangeOffset - fullScale / 2.0);

            pvoltData[i] -= zeroingOffset;
        }
    }
    /* field (OUTA, "${MON}V-Wf PP"):
     * final voltage waveform data, only copy effective number of samples
     * */
    memcpy((double *) precord->vala, pvoltData, pwf->nelm * sizeof(double));
    //dynamically change NELM field of '${PREFIX}VoltData-Wf'(OUTA)
    nSamples = pwf->nelm;//number of effective samples from INPA
    plink = &precord->outa;
    if (DB_LINK != plink->type)
        return -1;
    paddr = (DBADDR *) plink->value.pv_link.pvt;
    pwf = (waveformRecord *) paddr->precord;//pwf is now associated with OUTA
    pwf->nelm = nSamples;
    pwf->nord = nSamples;

    unsigned int *startP = (unsigned int *) precord->i;
    unsigned int *endP = (unsigned int *) precord->j;
    //reset starP and endP longout records; plink is INPI/INPJ now
    longoutRecord *plongout;
    if (*endP > pwf->nord)
    {
        *endP = pwf->nord;
        plink = &precord->inpj;
        assert((plink != NULL) && (DB_LINK == plink->type));
        paddr = (DBADDR *) plink->value.pv_link.pvt;
        plongout = (longoutRecord *) paddr->precord;
        plongout->val = *endP;
        //plongout->proc = 1;//can't cause the record to be processed
        errlogPrintf("endPoint should <= pwf->nord (%d), reset \n", pwf->nord);
    }
    if (*startP >= *endP)
    {
        *startP = 0;
        plink = &precord->inpi;
        assert((plink != NULL) && (DB_LINK == plink->type));
        paddr = (DBADDR *) plink->value.pv_link.pvt;
        plongout = (longoutRecord *) paddr->precord;
        plongout->val = *startP;
        errlogPrintf("startPoint should < endPoint, reset \n");
    }

    //calculate max, min, sum/integral, mean, std in the whole waveform
    processBasic(pvoltData, 0, pwf->nord, &max, &min, &sum, &ave, &std);
    //data analysis on part of the waveform (ROI, region of interest)
    processBasic(pvoltData, *startP, *endP, &maxROI, &minROI, &sumROI, &aveROI,
            &stdROI);

    //charge Q = V*T
    //sum *= sampleInterval;
    //sumROI *= sampleInterval;
    sum *= (1e9*ad->sampleInterval);
    sumROI *= (1e9*ad->sampleInterval);
    //ROI length: ns
    unsigned int ROISamples = *endP - *startP;
    *(double *) precord->valu = ROISamples * (1e9*ad->sampleInterval);

    /*search positive or negative pulse peaks: number of bunches,
     * normalized fill pattern, Max variation, individual bunch charge, etc.
     * find peaks and then sum values of at least 5 points.
     * in case of negative pulses: convert them to positive to keep the filling
     * pattern algorithm below suitable for either positive or negative beam
     * signal.HOWEVER,this algorithm doesn't work for the signal with both
     * negative and positive peaks, i.e. sinewave with -0.5V~+0.5V
     * */

    //memset consumes lots of CPU load (~100%) if using noa=1000000
    //memset(pfillPattern, 0, precord->noa * sizeof(double));
    memset(pfillPattern, 0, pwf->nelm * sizeof(double));
    for (i = 0; i < pwf->nord; i++) // nord == nelm: see acqiris_drv_wf.cpp
    {
        if (pvoltData[i] < 0.0)
            pvoltData[i] = (-pvoltData[i]);
    }
    //nbrSampleForSum is odd and >= 5: see ${PREFIX}NbrSamplesForSum-Calc_
    for (i = (nbrSampleForSum - 1) / 2; i < (pwf->nelm - (nbrSampleForSum - 1)
            / 2); i++)
    {
        if (fabs(pvoltData[i]) > fabs(peakThreshold))
        {
            //what if there are 2 identical peaks? Does the following work?
            if ((pvoltData[i - 1] < pvoltData[i]) && (pvoltData[i - 2]
                    < pvoltData[i]) && (pvoltData[i] >= pvoltData[i + 1])
                    && (pvoltData[i] >= pvoltData[i + 2]))
            {
                for (j = -(nbrSampleForSum - 1) / 2; j <= (nbrSampleForSum - 1)
                        / 2; j++)
                {
                    //integral voltage value of each bunch
                    pfillPattern[numBunch] += pvoltData[i + j];
                }
                //absolute charge of each bunch
                pfillPattern[numBunch] *= (coefBunchQ * (1e9
                        * ad->sampleInterval));
                numBunch++;
                pPeakIndex[k++] = i;
            }
        }//if (fabs(pvoltData[i]) > fabs(peakThreshold))
    }//for (i=(nbrSampleForSum-1)/2; i<(pwf->nelm -(nbrSampleForSum-1)/2); i++)

    //put integral (absolute bunch charge, 150-bunch for NSLS-2) into OUTL
    memcpy((double *) precord->vall, &pfillPattern[0],
            MAX_NUM_BUNCH * sizeof(double));
    //add up individual bunch charge to get the total charge of the bunch-train
    for (i = 0; i < numBunch; i++)
    {
        qSum += pfillPattern[i];
    }
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
    //above:memcpy((double *)precord->vala,pvoltData,pwf->nelm*sizeof(double));
    memcpy((double *) precord->valb, &max, precord->novb * sizeof(double));
    memcpy((double *) precord->valc, &min, precord->novc * sizeof(double));
    memcpy((double *) precord->vald, &ave, precord->novd * sizeof(double));
    memcpy((double *) precord->vale, &std, precord->nove * sizeof(double));
    memcpy((long *) precord->valf, &numBunch, precord->novf * sizeof(long));
    memcpy((double *) precord->valg, &pfillPattern[0],
            MAX_NUM_BUNCH * sizeof(double));
    memcpy((double *) precord->valh, &b2BMaxVar, precord->novh * sizeof(double));
    //not: memcpy((double *)precord->vali,&BunchQCalib,...);
    memcpy((long *) precord->valj, &maxQBunchNum, precord->novj * sizeof(long));
    memcpy((long *) precord->valk, &minQBunchNum, precord->novk * sizeof(long));
    //above:memcpy((double *)precord->vall,&pfillPattern[0], ...);
    memcpy((int *) precord->valm, &pPeakIndex[0], MAX_NUM_BUNCH * sizeof(int));
    memcpy((double *) precord->valn, &ad->realTrigRate,
            precord->novn * sizeof(double));
    //sum and sumROI is charge: sum of volts * (1e9*ad->sampleInterval)
    memcpy((double *) precord->valo, &sum, precord->novo * sizeof(double));
    memcpy((double *) precord->valp, &aveROI, precord->novp * sizeof(double));
    memcpy((double *) precord->valq, &maxROI, precord->novq * sizeof(double));
    memcpy((double *) precord->valr, &minROI, precord->novr * sizeof(double));
    memcpy((double *) precord->vals, &sumROI, precord->novs * sizeof(double));
    memcpy((double *) precord->valt, &stdROI, precord->novt * sizeof(double));
    //above: *(double *) precord->valu = ROISamples * (1e9*ad->sampleInterval);
    //aSub only has fields from A to U: use unused INP* instead of ONT*
    memcpy((double *) precord->k, &maxSum, precord->nok * sizeof(double));
    memcpy((double *) precord->l, &minSum, precord->nol * sizeof(double));
    memcpy((double *) precord->m, &qSum, precord->nom * sizeof(double));
    //printf("end of %s in acqirisAsubProcess()\n",precord->name);
    return (0);
}

/*see acqiris_hannel.db for INP/OUT fields
 * process buffered data: 60 samples (1-minute data for 1Hz injection)
 * */
static long
processBuf(aSubRecord *precord)
{
    assert(precord != NULL);

    unsigned j = 0;
    double sum = 0.0;
    double ave = 0.0;
    double std = 0.0;

    unsigned int nbrShots = *(unsigned int *) precord->b;
    //printf("nbrShots: %d \n", nbrShots);
    unsigned *pShots = (unsigned *) precord->c;
    double *pbufQ = (double *) precord->valc;
    if (*pShots < nbrShots)
    {
        //((double *) precord->valc)[*pShots] = *(double *) precord->a;
        pbufQ[*pShots] = *(double *) precord->a;
        (*pShots)++;
        //printf("*pShots is %d \n", *pShots);
    }
    else
    {
        *pShots = 0;
        for (j = 0; j < nbrShots; j++)
        {
            //sum += ((double *) precord->valc)[j];
            sum += pbufQ[j];
        }
        ave = sum / nbrShots;
        //printf("sum: %f, ave is %f \n", sum, ave);
        for (j = 0; j < nbrShots; j++)
        {
            std += (pbufQ[j] - ave) * (pbufQ[j] - ave);
        }
        std = sqrt(std / nbrShots);
        //printf("rmsNoise is %f \n", std);
        //OUTA, "${MON}StdVROIOverINOS-I PP"
        *(double *) precord->vala = std;
    }

    return (0);
}

/*see "${MON}TimeAxis-ASub_" in acqiris_module.db for INP/OUT links*/
static long
timeAxisAsubInit(aSubRecord *precord, processMethod process)
{
    if (timeAxisAsubInitialized)
        return (0);
    if (NULL == (ptimeAxis = (double *) malloc(precord->nova * sizeof(double))))
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

/*This routine is called only when nSamples or sampleLength is changed,
 *  see acqiris_module.db*/
static long
timeAxisAsubProcess(aSubRecord *precord)
{
    long nSample;
    double sampleLength;
    long i = 0;
    DBLINK *plink;
    DBADDR *paddr;
    waveformRecord *pwf;
    double sampleInterval;

    //input links: number of samples(data points), sample length (N ns)
    memcpy(&nSample, (long *) precord->a, precord->noa * sizeof(long));
    memcpy(&sampleLength, (double *) precord->b, precord->nob * sizeof(double));
    //sampleInterval is used for charge calculation in acqirisAsubProcess()
    sampleInterval = sampleLength / nSample;
    /*get effective number of samples('NELM') in the waveform
     * instead of 'NOA'(max. samples/ch) for data analysis
     * */
    plink = &precord->outa;
    if (DB_LINK != plink->type)
        return -1;
    paddr = (DBADDR *) plink->value.pv_link.pvt;
    pwf = (waveformRecord *) paddr->precord;
    pwf->nelm = nSample;
    //printf("number of effective samples(samples/ch): %d \n", pwf->nelm)
    for (i = 0; i < pwf->nelm; i++)
    {
        ptimeAxis[i] = i * (sampleLength / nSample);
    }
    memcpy((double *) precord->vala, ptimeAxis, pwf->nelm * sizeof(double));

    //printf("end of %s in timeAxisAsubProcess()\n",precord->name);
    return (0);
}

epicsRegisterFunction(acqirisAsubInit)
;
epicsRegisterFunction(acqirisAsubProcess)
;
epicsRegisterFunction(processBuf)
;
epicsRegisterFunction(timeAxisAsubInit)
;
epicsRegisterFunction(timeAxisAsubProcess)
;
