#ifndef _ACQIRISEXDATATYPES_H
#define _ACQIRISEXDATATYPES_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  AcqirisExDataTypes.h        
//----------------------------------------------------------------------------------------
//  Copyright Agilent Technologies, Inc. 2008
//
//  $Id: AcqirisExDataTypes.h 32275 2008-10-06 09:27:38Z jpapale $
//
//  
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "AcqirisDataTypes.h"

/////////////////////////////////////////////////////////////
// Declarations for the Attribute based system
typedef enum 
{
    AqUnknownType = 0,
    AqLong,
    AqDouble, 
    AqString,
    AqInt64,
    nbrAqAttrType
} AqAttrType;

typedef enum 
{
    AqUnknownAccess = 0,
    AqRO,                    //!< Read Only
    AqWO,                    //!< Write Only
    AqRW,                    //!< Read/Write
    nbrAqAttrAccess
} AqAttrAccess;

typedef enum 
{
    AqUnknownCategory = 0, 
    AqInstrument,          //!< Attribute is at instrument level
    AqChannel,             //!< Attribute is at channel level
    AqControlIO,           //!< Attribute is for Control I/O
    AqAverager,            //!< Attribute is for Instrument averager capability (if any)
    AqLogicalDevice,       //!< Attribute is related to a Logical Device, (for ie an FPGA)
    AqInternal,            //!< Attribute is a driver internal value (private)
    AqHardwareRegister,    //!< Attribute is a hardware register (private)
    AqRegisterEx,          //!< Attribute is an extended hardware register (private)
    AqRegisterExBitField,  //!< Attribute is a bitfield of an extended hardware register (private)
    AqProfiling,           //!< Attribute is a profiling dedicated attribute (private)
    nbrAqAttrCategory
} AqAttrCategory;

//! The order is determinant. More visibles must have highest number.
typedef enum 
{
    AqUnknownVisibility = 0, 
    AqPrivate,
    AqPublic,
    nbrAqAttrVisibility
} AqAttrVisibility;

typedef struct AqAttrDescStruct
{
    ViConstString           name;               //!< Attribute's name. Use this one for setting or getting an attribute.        
    ViSession               instrumentId;       //!< Instrument id who own this attribute
    AqAttrType              type;               //!< ie: Long, Double, String.
    AqAttrAccess            access;             //!< ie: GetOnly, GetSet.
    AqAttrCategory          category;           //!< ie: Instrument, Channel, System... etc.
    AqAttrVisibility        visibility;         //!< ie: Private, Public.
    ViInt32                 channel;            //!< Channel that own this attribute. If negative this number indicates 
                                                //!< an external channel. For ie: 1 = ch1, -1 = ext1.
                                                //!< If the attribute does not belong to a channel, this value is 0.
    ViInt32                 numberInCategory;   //!< Distinguish an attribute in same category. 
                                                //!< For category HardwareRegister, numberInCategory is the address
    ViInt32                 reserved1;          //!< Unused.
    ViInt32                 reserved2;          //!< Unused.
    ViBoolean               reserved3;          //!< Unused.
    ViBoolean               isStringCompatible; //!< Return '1' if this attribute can be manipulated through
                                                //!< 'SetAttrString/GetAttrString'.
    ViBoolean               hasARangeTable;     //!< Return '1' if this attribute has a range table (see getAttributeRangeTable())
    ViBoolean               isLocked;           //!< Return '1' if the driver's value is locked. (see setAttributexxxx())
    struct AqAttrDescStruct*    nextAttribute;  //!< Pointer to the next attribute. 
                                                //!< NULL if there isn't any next attribute. For ie we have reach 
                                                //!< the last.
} AqAttrDescriptor;
#endif// sentry
