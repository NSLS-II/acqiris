/* 
    Original Author: Perazzo, Amedeo
    Modified by Yong Hu: 10/29/2010
*****************************************************************************************
    acqiris_dev_wf(ao,etc.).cpp --> acqiris_*_record()(acqiris_dev_init.cpp) --> acqiris_*_record_specialized()(acqiris_drv_wf(ao,etc.).cpp)
    the templates 'acqiris_*_record' are implemented in acqiris_dev_init.cpp;
    the templates 'acqiris_*_record_specialized' are implemented in acqiris_drv_wf(ao,etc.).cpp;
*/

#ifndef ACQIRIS_DEV_HH
#define ACQIRIS_DEV_HH

#include <dbScan.h>
#include <link.h>

struct acqiris_record_t
{
  int module;
  int channel;
  char name[8];
  void* pvt;
};

typedef struct acqiris_record_t rec_t;

template<class T> int acqiris_init_record(T* record, DBLINK link);
template<class T> int acqiris_read_record(T* record);
template<class T> int acqiris_write_record(T* record);
template<class T> int acqiris_init_record_specialized(T* record);
template<class T> int acqiris_read_record_specialized(T* record);
template<class T> int acqiris_write_record_specialized(T* record);
//Yong HU: only implemented in acqiris_drv_wf.cpp
template<class T> IOSCANPVT acqiris_getioscanpvt_specialized(T* record);

#endif
