/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */
#ifndef CODEC_BLOSC_H
#define CODEC_BLOSC_H

#include <blosc.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/standardPVField.h>
#include <pv/ntscalar.h>
#include <pv/ntenum.h>

#include <shareLib.h>

namespace epics { namespace codec {

class BloscArgs
{
public:
    void * decompressAddr;
    size_t decompressSize;
    int elementSize;
};

class CodecBlosc;
typedef std::tr1::shared_ptr<CodecBlosc> CodecBloscPtr;
typedef std::tr1::weak_ptr<CodecBlosc> CodecBloscWPtr;

class epicsShareClass CodecBlosc
{
public:
    POINTER_DEFINITIONS(CodecBlosc);
    static CodecBloscPtr create();
    virtual ~CodecBlosc() {}

private:
    CodecBlosc();
    std::string message;
static epics::pvData::StructureConstPtr codecStructure;
static epics::pvData::StructureConstPtr createCodecStructure();
public:
static epics::pvData::StructureConstPtr getCodecStructure();
    bool compressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvDest,
        const epics::pvData::PVScalarArrayPtr & pvSource,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
    bool compressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvDest,
        BloscArgs *bloscArgs,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
    bool decompressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvSource,
        const epics::pvData::PVScalarArrayPtr & pvDest,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
    bool decompressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvSource,
        BloscArgs *bloscArgs,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
    std::string getMessage();
    
    void initCodecStructure(const epics::pvData::PVStructurePtr & pvStructure);
    bool setBloscArgs(
        const epics::pvData::PVScalarArrayPtr & pvScalarArray,
        BloscArgs *bloscArgs);
};

}}

#endif  /* CODEC_BLOSC_H */
