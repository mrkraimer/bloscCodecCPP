/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author Marty Kraimer
 * @date 2019.01
 */
#ifndef BLOSC_CODEC_H
#define BLOSC_CODEC_H

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

class BloscCodec;
typedef std::tr1::shared_ptr<BloscCodec> BloscCodecPtr;
typedef std::tr1::weak_ptr<BloscCodec> BloscCodecWPtr;
/**
 * @brief Code that implements blosc compress/decompress.
 *
 */
class epicsShareClass BloscCodec
{
public:
    POINTER_DEFINITIONS(BloscCodec);
    static BloscCodecPtr create();
    virtual ~BloscCodec() {}

private:
    BloscCodec();
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
        const void * decompressAddr, size_t decompressSize,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
    bool decompressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvSource,
        const epics::pvData::PVScalarArrayPtr & pvDest,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
   bool decompressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvSource,
        void * decompressAddr, size_t decompressSize,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
    std::string getMessage();
    
    void initCodecStructure(const epics::pvData::PVStructurePtr & pvStructure);
};

}}

#endif  /* BLOSC_CODEC_H */