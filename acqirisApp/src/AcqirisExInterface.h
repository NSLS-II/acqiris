///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AcqirisExInterface.h:    Common Driver Extended Function Declarations
//
//----------------------------------------------------------------------------------------
//  Copyright Acqiris, Inc. 2008
//
//  Purpose:    Declaration of Acqrs device driver extended API
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "vpptype.h"
#include "AcqirisExDataTypes.h"

// The following prefixes should be set outside of this file, typically by the
// AcqirisImport.h file, to specify the calling convention used for the function
// and whether it is exported or imported.
#if !defined(ACQ_DLL) || !defined(ACQ_CC)
#error AcqirisExInterface.h should not be included directly, please use AcqirisExImport.h instead.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////      
// General comments about the extende function prototypes:
//
// - All function calls require the argument 'instrumentID' in order to identify
//   the Acqiris card to which the call is directed.
//
// - All function calls return a status value of type 'ViStatus' with information about
//   the success or failure of the call.
//   The Acqiris specific values are defined in the AcqirisErrorCodes.h file.
//   The generic VISA ones are listed in the header file 'vpptype.h'.
//
// - If important parameters supplied by the user (e.g. an invalid instrumentID) are found
//   to be invalid, most functions do not execute and return an error code of the type
//   VI_ERROR_PARAMETERi, where i = 1, 2, ... corresponds to the argument number.
//
// - If the user attempts (with a function Acqrs_configXXXX) to set a instrument
//   parameter to a value which is outside of its acceptable range, the function
//   typically adapts the parameter to the closest available value and returns
//   ACQIRIS_WARN_SETUP_ADAPTED. The instrument parameters actually set can be retrieved
//   with the 'query' functions Acqrs_getXXXX.
//
// - Data are always returned through pointers to user-allocated variables or arrays.
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//! Return through "attrDescriptorP" the value of the attribute named "name".
/*! 'channel'   = 0   for instrument related attributes.
                = x   for channel related attributes, where 'x' is the channel number.

    'name'      = name of the attribute.

    'attrDescriptorP'    = the pointer where the value will be written.

   Returns one of the following ViStatus values:

    ACQIRIS_ERROR_ATTR_NOT_FOUND        if the attribute is not found.
    VI_SUCCESS                          otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getAttributeDescriptor(ViSession instrumentID, ViInt32 channel, ViConstString name, AqAttrDescriptor* attrDescriptorP); 


//! Return through "valueP" the value of the attribute named "name".
/*! 'channel'   = 0   for instrument related attributes.
                = x   for channel related attributes, where 'x' is the channel number.

    'name'      = name of the attribute.

    'valueP'    = the pointer where the value will be written.

   Returns one of the following ViStatus values:

    ACQIRIS_ERROR_ATTR_NOT_FOUND        if the attribute is not found. 
    ACQIRIS_ERROR_ATTR_WRONG_TYPE       if the attribute is found but not of the expected type.
    VI_SUCCESS                          otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getAttributeDouble(ViSession instrumentID, ViInt32 channel, ViConstString name, ViReal64* valueP); 


//! Return through "valueP" the value of the attribute named "name".
/*! 'channel'   = 0   for instrument related attributes.
                = x   for channel related attributes, where 'x' is the channel number.

    'name'      = name of the attribute.

    'valueP'    = the pointer where the value will be written.

   Returns one of the following ViStatus values:

    ACQIRIS_ERROR_ATTR_NOT_FOUND        if the attribute is not found. 
    ACQIRIS_ERROR_ATTR_WRONG_TYPE       if the attribute is found but not of the expected type.
    VI_SUCCESS                          otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getAttributeLong(ViSession instrumentID, ViInt32 channel, ViConstString name, ViInt32* valueP); 


//! Return through "rangeTableP" the range table of the attribute named "name".
/*! 'channel'   = 0   for instrument related attributes.
                = x   for channel related attributes, where 'x' is the channel number.

    'name'      = name of the attribute.

    'rangeTableP'    = the pointer where the value will be written.

   Returns one of the following ViStatus values:

    ACQIRIS_ERROR_ATTR_NOT_FOUND        if the attribute is not found. 
    ACQIRIS_ERROR_ATTR_WRONG_TYPE       if the attribute is found but not of the expected type.
    ACQIRIS_ERROR_NOT_SUPPORTED         if found but not implemented.
    VI_SUCCESS                          otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getAttributeRangeTable(ViSession instrumentID, ViInt32 channel, ViConstString name, IviRangeTablePtr rangeTableP); 


//! Return through "string" a copy of the string expression of the attribute named "name".
/*! 'channel'   = 0   for instrument related attributes.
                = x   for channel related attributes, where 'x' is the channel number.

    'name'      = name of the attribute.

    'string'    = the string where the value will be written.

    'bufferSize'= the size of the 'string' buffer in bytes.

   Returns one of the following ViStatus values:

    ACQIRIS_ERROR_ATTR_NOT_FOUND        if the attribute is not found. 
    ACQIRIS_ERROR_ATTR_WRONG_TYPE       if the attribute is found but not of the expected type.
    ACQIRIS_ERROR_BUFFER_OVERFLOW       if 'bufferSize' is too small.
    VI_SUCCESS                          otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_getAttributeString(ViSession instrumentID, ViInt32 channel, ViConstString name, ViString string, ViInt32 bufferSize); 


//! Set through "value" the value of the attribute named "name". 
/*! 'channel'   = 0   for instrument related attributes.
                = x   for channel related attributes, where 'x' is the channel number.

    'name'      = name of the attribute.

    'value'     = value of the attribute.

   Returns one of the following ViStatus values:

    ACQIRIS_ERROR_ATTR_NOT_FOUND        if the attribute is not found. 
    ACQIRIS_ERROR_ATTR_WRONG_TYPE       if the attribute is found but not of the expected type.
    ACQIRIS_ERROR_ATTR_IS_READ_ONLY     if the attribute is found but not writable.
    VI_SUCCESS                          otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_setAttributeDouble(ViSession instrumentID, ViInt32 channel, ViConstString name, ViReal64 value); 


//! Set through "value" the value of the attribute named "name". 
/*! 'channel'   = 0   for instrument related attributes.
                = x   for channel related attributes, where 'x' is the channel number.

    'name'      = name of the attribute.

    'value'     = value of the attribute.

   Returns one of the following ViStatus values:

    ACQIRIS_ERROR_ATTR_NOT_FOUND        if the attribute is not found. 
    ACQIRIS_ERROR_ATTR_WRONG_TYPE       if the attribute is found but not of the expected type.
    ACQIRIS_ERROR_ATTR_IS_READ_ONLY     if the attribute is found but not writable.
    VI_SUCCESS                          otherwise. */
ACQ_DLL ViStatus ACQ_CC Acqrs_setAttributeLong(ViSession instrumentID, ViInt32 channel, ViConstString name, ViInt32 value); 

