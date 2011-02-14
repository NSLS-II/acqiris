///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AcqirisG4Interface.h:    Generator Driver Function Declarations
//
//----------------------------------------------------------------------------------------
//  Copyright Acqiris, Inc. 2006, 2007-2008
//
//  Purpose:    Declaration of $prefixG4$ device driver API
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "vpptype.h"
#include "AcqirisDataTypes.h"

// The following prefixes should be set outside of this file, typically by the
// AcqirisG4Import.h file, to specify the calling convention used for the function
// and whether it is exported or imported.
#if !defined(ACQ_DLL) || !defined(ACQ_CC)
#error AcqirisG4Interface.h should not be included directly, please use AcqirisG4Import.h instead.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////      
// General comments about the function prototypes:
//
// - All function calls require the argument 'instrumentID' in order to identify
//   the Acqiris Digitizer card to which the call is directed.
//   The only exceptions are the initialization functions '$prefixG4$_getNbrPhysicalInstruments',
//   '$prefixG4$_setSimulationOptions', '$prefixG4$_init' and '$prefixG4$_InitWithOptions'.
//   The last two functions actually return instrument identifiers at initialization time,
//   for subsequent use in the other function calls.
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
// - If the user attempts (with a function $prefixG4$_configXXXX) to set a digitizer
//   parameter to a value which is outside of its acceptable range, the function
//   typically adapts the parameter to the closest available value and returns
//   ACQIRIS_WARN_SETUP_ADAPTED. The digitizer parameters actually set can be retrieved
//   with the 'query' functions $prefixG4$_getXXXX.
//
// - Data are always returned through pointers to user-allocated variables or arrays.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

ACQ_DLL ViStatus _VI_FUNC AqG4_calibrate(ViSession instrumentID); 
// Performs an auto-calibration of the instrument


ACQ_DLL ViStatus _VI_FUNC AqG4_calibrateEx(ViSession instrumentID, ViInt32 type,
                         ViInt32 modifier, ViInt32 flags); 
// Performs an auto-calibration of the instrument with parameters
// modifier=2: calibrates using external clock


///////////////////////////////////////////////////////////////////////////////////////////////////
// From new version of 'visa.h'. Specified here, until we update it.
#ifndef VI_SUCCESS_MAX_CNT
#define VI_SUCCESS_MAX_CNT  (0x3FFF0006L) /* 3FFF0006,  1073676294 */
#endif //VI_SUCCESS_MAX_CNT


///////////////////////////////////////////////////////////////////////////////////////////////////
// IVI driver inherent capabilities

#define IVI_INHERENT_ERROR_BASE (0xBFFA0000)
#define IVI_INHERENT_WARN_BASE (0x3FFA0000)
#define IVI_INHERENT_ATTR_BASE (1050000)

#define IVI_ERROR_ATTR_NOT_WRITEABLE (IVI_INHERENT_ERROR_BASE + 0x0D)
#define IVI_ERROR_ATTR_NOT_READABLE (IVI_INHERENT_ERROR_BASE + 0x0E)
#define IVI_ERROR_UNKNOWN_NAME_IN_SELECTOR (IVI_INHERENT_ERROR_BASE + 0x65)
#define IVI_UNKNOWN_PHYSICAL_IDENTIFIER (IVI_INHERENT_ERROR_BASE + 0x67)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ClearError(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ClearInterchangeWarnings(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_close(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_Disable(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_error_message(ViSession vi, ViStatus errorCode, ViChar errorMessage[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_error_query(ViSession vi, ViInt32 *errorCodeP, ViChar errorMessage[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetAttributeViInt32(ViSession vi, ViConstString repCapId, ViAttr attrId, ViInt32 *valueP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetAttributeViReal64(ViSession vi, ViConstString repCapId, ViAttr attrId, ViReal64 *valueP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetAttributeViBoolean(ViSession vi, ViConstString repCapId, ViAttr attrId, ViBoolean *valueP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetAttributeViSession(ViSession vi, ViConstString repCapId, ViAttr attrId, ViSession *valueP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetAttributeViString(ViSession vi, ViConstString repCapId, ViAttr attrId, ViInt32 valueSize, ViChar valueP[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetError(ViSession vi, ViStatus *errorCodeP, ViInt32 errorDescSize, ViChar errorDescP[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetNextCoercionRecord(ViSession vi, ViInt32 coercionRecordSize, ViChar coercionRecordP[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetNextInterchangeWarning(ViSession vi, ViInt32 interchangeWarningSize, ViChar interchangeWarningP[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetSpecificDriverCHandle(ViSession vi, ViSession *handleSpecificP);

///////////////////////////////////////////////////////////////////////////////////////////////////
// There is a problem using this IUnknown type. On windows, using the typedef from void generates
// an internal compiler error.   The real definition found is typdefe interface IUnknown IUnknown;
// but it is not valid in C++ only (no COM). Thus this function is removed
#ifdef _WINDOWS
#   define IUnknown void
#else
    typedef void IUnknown;
#endif

ACQ_DLL ViStatus _VI_FUNC AqG4_GetSpecificDriverIUnknownPtr(ViSession vi, IUnknown **iunknownPtrP);

#ifdef _WINDOWS
#   undef IUnknown
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_InitWithOptions(ViRsrc resourceName, ViBoolean idQuery, ViBoolean reset, ViConstString optionsStringP, ViSession *viP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_init(ViRsrc resourceName, ViBoolean idQuery, ViBoolean reset, ViSession *viP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_InvalidateAllAttributes(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_LockSession(ViSession vi, ViBoolean *callerHasLockP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_reset(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ResetInterchangeCheck(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ResetWithDefaults(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_revision_query(ViSession vi, ViChar driverRevP[], ViChar instrRevP[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_self_test(ViSession vi, ViInt16 *testResultP, ViChar testMessageP[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_SetAttributeViInt32(ViSession vi, ViConstString repCapId, ViAttr attrId, ViInt32 value);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_SetAttributeViReal64(ViSession vi, ViConstString repCapId, ViAttr attrId, ViReal64 value);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_SetAttributeViString(ViSession vi, ViConstString repCapId, ViAttr attrId, ViConstString value);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_SetAttributeViBoolean(ViSession vi, ViConstString repCapId, ViAttr attrId, ViBoolean value);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_SetAttributeViSession(ViSession vi, ViConstString repCapId, ViAttr attrId, ViSession value);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_UnlockSession(ViSession vi, ViBoolean *callerHasLockP);

/*
///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_(ViSession vi);
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenBase capability group

#define IVI_CLASS_ATTR_BASE (1250000)

#define IVIFGEN_ATTR_CHANNEL_COUNT (IVI_INHERENT_ATTR_BASE + 203)
#define IVIFGEN_ATTR_OUTPUT_MODE (IVI_CLASS_ATTR_BASE + 1)
#define IVIFGEN_ATTR_REF_CLOCK_SOURCE (IVI_CLASS_ATTR_BASE + 2)
#define IVIFGEN_ATTR_OUTPUT_ENABLED (IVI_CLASS_ATTR_BASE + 3)
#define IVIFGEN_ATTR_OUTPUT_IMPEDANCE (IVI_CLASS_ATTR_BASE + 4)
#define IVIFGEN_ATTR_OPERATION_MODE (IVI_CLASS_ATTR_BASE + 5)
#define IVIFGEN_ATTR_OUTPUT_INTERPOLATED (IVI_CLASS_ATTR_BASE + 99)
#define IVIFGEN_ATTR_EXT_CLOCK_FREQUENCY (IVI_CLASS_ATTR_BASE + 98)
#define IVIFGEN_ATTR_EXT_CLOCK_THRESHOLD (IVI_CLASS_ATTR_BASE + 97)

// Defined valued for attribute IVIFGEN_ATTR_OUTPUT_MODE
#define IVIFGEN_VAL_OUTPUT_FUNC (0)
#define IVIFGEN_VAL_OUTPUT_ARB (1)
#define IVIFGEN_VAL_OUTPUT_SEQ (2)
#define IVIFGEN_VAL_OUT_MODE_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_OUTPUT_FPGA (501)
#define IVIFGEN_VAL_OUT_MODE_SPECIFIC_EXT_BASE (1000)
// Defined valued for attribute IVIFGEN_ATTR_OPERATION_MODE
#define IVIFGEN_VAL_OPERATE_CONTINUOUS (0)
#define IVIFGEN_VAL_OPERATE_BURST (1)
#define IVIFGEN_VAL_OP_MODE_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_OP_MODE_SPECIFIC_EXT_BASE (1000)
// Defined values for attribute IVIFGEN_ATTR_REF_CLOCK_SOURCE
#define IVIFGEN_VAL_REF_CLOCK_INTERNAL (0)
#define IVIFGEN_VAL_REF_CLOCK_EXTERNAL (1)
#define IVIFGEN_VAL_REF_CLOCK_RTSI_CLOCK (101)
#define IVIFGEN_VAL_CLK_SRC_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_REF_CLOCK_EXTERNAL_CLOCK (501)
#define IVIFGEN_VAL_REF_CLOCK_START_STOP_CLOCK (502)
#define IVIFGEN_VAL_CLK_SRC_SPECIFIC_EXT_BASE (1000)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_AbortGeneration(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureOperationMode(ViSession vi, ViConstString channelName, ViInt32 operationMode);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureOutputEnabled(ViSession vi, ViConstString channelName, ViBoolean enabled);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureOutputImpedance(ViSession vi, ViConstString channelName, ViReal64 impedance);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureOutputMode(ViSession vi, ViInt32 outputMode);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureRefClockSource(ViSession vi, ViInt32 source);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_GetChannelName(ViSession vi, ViInt32 index, ViInt32 nameSize, ViChar nameP[]);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_InitiateGeneration(ViSession vi);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenStdFunc extension group

#define IVIFGEN_ATTR_FUNC_WAVEFORM (IVI_CLASS_ATTR_BASE + 101)
#define IVIFGEN_ATTR_FUNC_AMPLITUDE (IVI_CLASS_ATTR_BASE + 102)
#define IVIFGEN_ATTR_FUNC_DC_OFFSET (IVI_CLASS_ATTR_BASE + 103)
#define IVIFGEN_ATTR_FUNC_FREQUENCY (IVI_CLASS_ATTR_BASE + 104)
#define IVIFGEN_ATTR_FUNC_START_PHASE (IVI_CLASS_ATTR_BASE + 105)
#define IVIFGEN_ATTR_FUNC_DUTY_CYCLE_HIGH (IVI_CLASS_ATTR_BASE + 106)

// Defined values for attribute IVIFGEN_ATTR_FUNC_WAVEFORM
#define IVIFGEN_VAL_WFM_SINE (1)
#define IVIFGEN_VAL_WFM_SQUARE (2)
#define IVIFGEN_VAL_WFM_TRIANGLE (3)
#define IVIFGEN_VAL_WFM_RAMP_UP (4)
#define IVIFGEN_VAL_WFM_RAMP_DOWN (5)
#define IVIFGEN_VAL_WFM_DC (6)
#define IVIFGEN_VAL_WFM_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_WFM_SPECIFIC_EXT_BASE (1000)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureStandardWaveform(ViSession vi, ViConstString channelName, ViInt32 waveform, ViReal64 amplitude, ViReal64 offset, ViReal64 frequency, ViReal64 startPhase);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenArbWfm extension group

#define IVIFGEN_ATTR_ARB_WAVEFORM_HANDLE (IVI_CLASS_ATTR_BASE + 201)
#define IVIFGEN_ATTR_ARB_GAIN (IVI_CLASS_ATTR_BASE + 202)
#define IVIFGEN_ATTR_ARB_OFFSET (IVI_CLASS_ATTR_BASE + 203)
#define IVIFGEN_ATTR_ARB_SAMPLE_RATE (IVI_CLASS_ATTR_BASE + 204)
#define IVIFGEN_ATTR_MAX_NUM_WAVEFORMS (IVI_CLASS_ATTR_BASE + 205)
#define IVIFGEN_ATTR_WAVEFORM_QUANTUM (IVI_CLASS_ATTR_BASE + 206)
#define IVIFGEN_ATTR_MIN_WAVEFORM_SIZE (IVI_CLASS_ATTR_BASE + 207)
#define IVIFGEN_ATTR_MAX_WAVEFORM_SIZE (IVI_CLASS_ATTR_BASE + 208)

// Defined values for waveformHandle parameter for function IviFgen_ClearArbWaveform
#define IVIFGEN_VAL_ALL_WAVEFORMS (-1)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ClearArbWaveform(ViSession vi, ViInt32 handle);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureArbWaveform(ViSession vi, ViConstString channelName, ViInt32 handle, ViReal64 gain, ViReal64 offset);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureSampleRate(ViSession vi, ViReal64 sampleRate);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_CreateArbWaveform(ViSession vi, ViInt32 size, ViReal64 dataP[], ViInt32 *handleP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_QueryArbWfmCapabilities(ViSession vi, ViInt32 *maxNumWfmsP, ViInt32 *wfmQuantumP, ViInt32 *minwfmSizeP, ViInt32 *maxWfmSizeP);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenArbFrequency extension group

#define IVIFGEN_ATTR_ARB_FREQUENCY (IVI_CLASS_ATTR_BASE + 209)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureArbFrequency(ViSession vi, ViConstString channelName, ViReal64 frequency);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenArbSeq extension group

#define IVIFGEN_ATTR_ARB_SEQUENCE_HANDLE (IVI_CLASS_ATTR_BASE + 211)
#define IVIFGEN_ATTR_MAX_NUM_SEQUENCES (IVI_CLASS_ATTR_BASE + 212)
#define IVIFGEN_ATTR_MIN_SEQUENCE_LENGTH (IVI_CLASS_ATTR_BASE + 213)
#define IVIFGEN_ATTR_MAX_SEQUENCE_LENGTH (IVI_CLASS_ATTR_BASE + 214)
#define IVIFGEN_ATTR_MAX_LOOP_COUNT (IVI_CLASS_ATTR_BASE + 215)

// Defined values for sequenceHandle parameter for function IviFgen_ClearArbSequence
#define IVIFGEN_VAL_ALL_SEQUENCES (-1)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ClearArbMemory(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ClearArbSequence(ViSession vi, ViInt32 handle);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureArbSequence(ViSession vi, ViConstString channelName, ViInt32 handle, ViReal64 gain, ViReal64 offset);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_CreatArbSequence(ViSession vi, ViInt32 length, ViInt32 wfvHandleP[], ViInt32 loopCountP[], ViInt32 *handleP);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_QueryArbSeqCapabilities(ViSession vi, ViInt32 *maxNumSegsP, ViInt32 *minSeqLengthP, ViInt32 *maxSegLengthP, ViInt32 *maxLoopCountP);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenTrigger extension group

#define IVIFGEN_ATTR_TRIGGER_SOURCE (IVI_CLASS_ATTR_BASE + 302)

// Defined values for attribute IVIFGEN_ATTR_TRIGGER_SOURCE
#define IVIFGEN_VAL_EXTERNAL (1)
#define IVIFGEN_VAL_SOFTWARE_TRIG (2)
#define IVIFGEN_VAL_INTERNAL_TRIGGER (3)
#define IVIFGEN_VAL_TTL0 (111)
#define IVIFGEN_VAL_TTL1 (112)
#define IVIFGEN_VAL_TTL2 (113)
#define IVIFGEN_VAL_TTL3 (114)
#define IVIFGEN_VAL_TTL4 (115)
#define IVIFGEN_VAL_TTL5 (116)
#define IVIFGEN_VAL_TTL6 (117)
#define IVIFGEN_VAL_TTL7 (118)
#define IVIFGEN_VAL_ECL0 (119)
#define IVIFGEN_VAL_ECL1 (120)
#define IVIFGEN_VAL_PXI_STAR (131)
#define IVIFGEN_VAL_RTSI_0 (141)
#define IVIFGEN_VAL_RTSI_1 (142)
#define IVIFGEN_VAL_RTSI_2 (143)
#define IVIFGEN_VAL_RTSI_3 (144)
#define IVIFGEN_VAL_RTSI_4 (145)
#define IVIFGEN_VAL_RTSI_5 (146)
#define IVIFGEN_VAL_RTSI_6 (147)
#define IVIFGEN_VAL_TRIG_SRC_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_TRIG_SRC_SPECIFIC_EXT_BASE (1000)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureTriggerSource(ViSession vi, ViConstString channelName, ViInt32 source);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenInternalTrigger extension group

#define IVIFGEN_ATTR_INTERNAL_TRIGGER_RATE (IVI_CLASS_ATTR_BASE + 310)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureInternalTriggerRate(ViSession vi, ViReal64 rate);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenSoftwareTrigger extension group

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_SendSoftwareTrigger(ViSession vi);

///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenBurst extension group

#define IVIFGEN_ATTR_BURST_COUNT (IVI_CLASS_ATTR_BASE + 350)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureBurstCount(ViSession vi, ViConstString channelName, ViInt32 count);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenModulateAM extension group

#define IVIFGEN_ATTR_AM_ENABLED (IVI_CLASS_ATTR_BASE + 401)
#define IVIFGEN_ATTR_AM_SOURCE (IVI_CLASS_ATTR_BASE + 402)
#define IVIFGEN_ATTR_AM_INTERNAL_DEPTH (IVI_CLASS_ATTR_BASE + 403)
#define IVIFGEN_ATTR_AM_INTERNAL_WAVEFORM (IVI_CLASS_ATTR_BASE + 404)
#define IVIFGEN_ATTR_AM_INTERNAL_FREQUENCY (IVI_CLASS_ATTR_BASE + 405)

// Defined values for attribute IVIFGEN_ATTR_AM_SOURCE
#define IVIFGEN_VAL_AM_INTERNAL (0)
#define IVIFGEN_VAL_AM_EXTERNAL (1)
#define IVIFGEN_VAL_AM_SOURCE_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_AM_SOURCE_SPECIFIC_EXT_BASE (1000)

// Defined values for attribute IVIFGEN_ATTR_AM_INTERNAL_WAVEFORM
#define IVIFGEN_VAL_AM_INTERNAL_SINE (1)
#define IVIFGEN_VAL_AM_INTERNAL_SQUARE (2)
#define IVIFGEN_VAL_AM_INTERNAL_TRIANGLE (3)
#define IVIFGEN_VAL_AM_INTERNAL_RAMP_UP (4)
#define IVIFGEN_VAL_AM_INTERNAL_RAMP_DOWN (5)
#define IVIFGEN_VAL_AM_INTERNAL_WFM_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_AM_INTERNAL_WFM_SPECIFIC_EXT_BASE (1000)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureAMEnabled(ViSession vi, ViConstString channelName, ViBoolean enabled);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureAMInternal(ViSession vi, ViReal64 depth, ViInt32 waveform, ViReal64 frequency);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureAMSource(ViSession vi, ViConstString channelName, ViInt32 source);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenModulateFM extension group

#define IVIFGEN_ATTR_FM_ENABLED (IVI_CLASS_ATTR_BASE + 501)
#define IVIFGEN_ATTR_FM_SOURCE (IVI_CLASS_ATTR_BASE + 502)
#define IVIFGEN_ATTR_FM_INTERNAL_DEVIATION (IVI_CLASS_ATTR_BASE + 503)
#define IVIFGEN_ATTR_FM_INTERNAL_WAVEFORM (IVI_CLASS_ATTR_BASE + 504)
#define IVIFGEN_ATTR_FM_INTERNAL_FREQUENCY (IVI_CLASS_ATTR_BASE + 505)

// Defined values for attribute IVIFGEN_ATTR_FM_SOURCE
#define IVIFGEN_VAL_FM_INTERNAL (0)
#define IVIFGEN_VAL_FM_EXTERNAL (1)
#define IVIFGEN_VAL_FM_SOURCE_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_FM_SOURCE_SPECIFIC_EXT_BASE (1000)

// Defined values for attribute IVIFGEN_ATTR_FM_INTERNAL_WAVEFORM
#define IVIFGEN_VAL_FM_INTERNAL_SINE (1)
#define IVIFGEN_VAL_FM_INTERNAL_SQUARE (2)
#define IVIFGEN_VAL_FM_INTERNAL_TRIANGLE (3)
#define IVIFGEN_VAL_FM_INTERNAL_RAMP_UP (4)
#define IVIFGEN_VAL_FM_INTERNAL_RAMP_DOWN (5)
#define IVIFGEN_VAL_FM_INTERNAL_WFM_CLASS_EXT_BASE (500)
#define IVIFGEN_VAL_FM_INTERNAL_WFM_SPECIFIC_EXT_BASE (1000)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureFMEnabled(ViSession vi, ViConstString channelName, ViBoolean enabled);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureFMInternal(ViSession vi, ViReal64 deviation, ViInt32 waveform, ViReal64 frequency);

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_ConfigureFMSource(ViSession vi, ViConstString channelName, ViInt32 source);


///////////////////////////////////////////////////////////////////////////////////////////////////
// IviFgenFpgaFunc extension group

#define IVIFGEN_ATTR_FPGA_WAVEFORM (IVI_CLASS_ATTR_BASE + 601)
#define IVIFGEN_ATTR_FPGA_AMPLITUDE (IVI_CLASS_ATTR_BASE + 602)
#define IVIFGEN_ATTR_FPGA_DC_OFFSET (IVI_CLASS_ATTR_BASE + 603)
#define IVIFGEN_ATTR_FPGA_FREQUENCY (IVI_CLASS_ATTR_BASE + 604)
#define IVIFGEN_ATTR_FPGA_START_PHASE (IVI_CLASS_ATTR_BASE + 605)
#define IVIFGEN_ATTR_FPGA_DUTY_CYCLE_HIGH (IVI_CLASS_ATTR_BASE + 606)
#define IVIFGEN_ATTR_FPGA_MODE (IVI_CLASS_ATTR_BASE + 610)

// Defined values for attribute IVIFGEN_ATTR_FPGA_MODE
#define IVIFGEN_VAL_FPGA_MODE_DDS (0)
#define IVIFGEN_VAL_FPGA_MODE_REPEAT (1)
#define IVIFGEN_VAL_FPGA_MODE_MANUAL (2)
#define IVIFGEN_VAL_FPGA_MODE_DUALTONE (3)

///////////////////////////////////////////////////////////////////////////////////////////////////
ACQ_DLL ViStatus _VI_FUNC AqG4_WaitForEndOfProcessing(ViSession vi, ViInt32 timeoutMilliseconds);


///////////////////////////////////////////////////////////////////////////////////////////////////




