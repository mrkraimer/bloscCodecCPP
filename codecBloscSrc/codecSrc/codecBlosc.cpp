/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */


#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>

#include <shareLib.h>
#include <epicsExport.h>
#include <pv/codecBlosc.h>

namespace epics { namespace codec {

using namespace epics::pvData;
using namespace std;
using std::tr1::static_pointer_cast;

StructureConstPtr CodecBlosc::codecStructure = CodecBlosc::createCodecStructure();

StructureConstPtr CodecBlosc::createCodecStructure()
{
 FieldCreatePtr fieldCreate = getFieldCreate();
    StandardFieldPtr standardField = getStandardField();

    StructureConstPtr bloscArgs = fieldCreate->createFieldBuilder()->
        add("compressedSize",pvInt)->
        add("decompressedSize",pvInt)->
        add("level",pvInt)->
        add("compressor",standardField->enumerated()) ->
        add("shuffle",standardField->enumerated()) ->
        add("threads",pvInt)->
        createStructure();

    StructureConstPtr  topStructure = fieldCreate->createFieldBuilder()->
         addArray("value",pvUByte)->
         add("alarm",standardField->alarm()) ->
         add("timeStamp",standardField->timeStamp()) -> 
         add("channelName",pvString)->
         add("channelType",pvString)->
         add("command",standardField->enumerated()) ->
         add("codecName",pvString)->
         add("bloscArgs",bloscArgs) ->
         createStructure();
    return topStructure;
}

CodecBloscPtr CodecBlosc::create()
{
    CodecBloscPtr codecBlosc(new CodecBlosc());
    return codecBlosc;
}

CodecBlosc::CodecBlosc(){}

bool CodecBlosc::compressBlosc(
        const PVUByteArrayPtr & pvDest,
        const PVScalarArrayPtr & pvSource,
        const PVStructurePtr & pvBloscArgs)
{
    BloscArgs bloscArgs;
    bool result = setBloscArgs(pvSource,&bloscArgs);
    if(result) result = compressBlosc(pvDest,&bloscArgs,pvBloscArgs);
    return result;
}

bool CodecBlosc::setBloscArgs(
    const PVScalarArrayPtr & pvSource,
    BloscArgs *bloscArgs)
{
    ScalarType scalarType = pvSource->getScalarArray()->getElementType();
    switch(scalarType)
    {
    case pvByte:
        {
             PVByteArrayPtr pvSrc = static_pointer_cast<PVByteArray>(pvSource);
             PVByteArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 1;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength();
             return true;
        }
    case pvUByte:
        {
             PVUByteArrayPtr pvSrc = static_pointer_cast<PVUByteArray>(pvSource);
             PVUByteArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 1;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength();
             return true;
        }
    case pvShort:
        {
             PVShortArrayPtr pvSrc = static_pointer_cast<PVShortArray>(pvSource);
             PVShortArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 2;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength()*bloscArgs->elementSize;
             return true;
        }
    case pvUShort:
        {
             PVUShortArrayPtr pvSrc = static_pointer_cast<PVUShortArray>(pvSource);
             PVUShortArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 2;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength()*bloscArgs->elementSize;
             return true;
        }
    case pvInt:
        {
             PVIntArrayPtr pvSrc = static_pointer_cast<PVIntArray>(pvSource);
             PVIntArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 4;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength()*bloscArgs->elementSize;
             return true;
        }
    case pvUInt:
        {
             PVUIntArrayPtr pvSrc = static_pointer_cast<PVUIntArray>(pvSource);
             PVUIntArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 4;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength()*bloscArgs->elementSize;
             return true;
        }
    case pvLong:
        {
             PVLongArrayPtr pvSrc = static_pointer_cast<PVLongArray>(pvSource);
             PVLongArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 8;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength()*bloscArgs->elementSize;
             return true;
        }
    case pvULong:
        {
             PVULongArrayPtr pvSrc = static_pointer_cast<PVULongArray>(pvSource);
             PVULongArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 8;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength()*bloscArgs->elementSize;
             return true;
        }
    case pvFloat:
        {
             PVFloatArrayPtr pvSrc = static_pointer_cast<PVFloatArray>(pvSource);
             PVFloatArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 4;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength()*bloscArgs->elementSize;
             return true;
        }
    case pvDouble:
        {
             PVDoubleArrayPtr pvSrc = static_pointer_cast<PVDoubleArray>(pvSource);
             PVDoubleArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             bloscArgs->elementSize = 8;
             bloscArgs->decompressAddr = (void *)sourcedata.data();
             bloscArgs->decompressSize = pvSource->getLength()*bloscArgs->elementSize;
             return true;
        }
    default:
        string mess("CodecBlosc::setBloscArgs pvType ");
        mess += scalarType;
        mess += " not supported";
        message = mess;
        return false;
    }
}

bool CodecBlosc::compressBlosc(
    const epics::pvData::PVUByteArrayPtr & pvDest,
    BloscArgs *bloscArgs,
    const PVStructurePtr & pvBloscArgs)
{
    int clevel = pvBloscArgs->getSubField<PVInt>("level")->get();
    int doshuffle = pvBloscArgs->getSubField<PVInt>("shuffle.index")->get();
    size_t typesize = 1;
    size_t nbytes = bloscArgs->decompressSize;
    const void* src = (const void *)bloscArgs->decompressAddr;
    size_t destsize = nbytes + BLOSC_MAX_OVERHEAD;
    pvDest->setLength(destsize);
    PVUByteArray::const_svector destdata;
    pvDest->getAs(destdata);   
    void *dest = (void *)destdata.data();
    PVStructurePtr pvCompressor = pvBloscArgs->getSubField<PVStructure>("compressor");
    int index = pvCompressor->getSubField<PVInt>("index")->get();
    PVStringArrayPtr pvChoices = pvCompressor->getSubField<PVStringArray>("choices");
    PVStringArray::const_svector choices;
    pvChoices->getAs(choices);
    string choice = choices[index];
    const char* compressor = choice.c_str();
    size_t blocksize = 0;
    int numinternalthreads = 1;

    int result = blosc_compress_ctx(
        clevel,doshuffle,typesize,
        nbytes,src,dest,
        destsize,compressor,
        blocksize,numinternalthreads);
    if(result>0) {
        pvDest->setLength(result);
        pvBloscArgs->getSubField<PVInt>("compressedSize")->put(result);
        pvBloscArgs->getSubField<PVInt>("decompressedSize")->put(bloscArgs->decompressSize);
        return true;
    }
    return false;
}

bool CodecBlosc::decompressBlosc(
    const epics::pvData::PVUByteArrayPtr & pvSource,
    const epics::pvData::PVScalarArrayPtr & pvDestination,
    const epics::pvData::PVStructurePtr & pvBloscArgs)
{
    BloscArgs bloscArgs;
    bool result = setBloscArgs(pvDestination,&bloscArgs);
    if(result) result = decompressBlosc(pvSource,&bloscArgs,pvBloscArgs);
    return result;
}
bool CodecBlosc::decompressBlosc(
    const epics::pvData::PVUByteArrayPtr & pvSource,
    BloscArgs *bloscArgs,
    const epics::pvData::PVStructurePtr & pvBloscArgs)
{
    PVUByteArray::const_svector sourcedata;
    pvSource->getAs(sourcedata);   
    void *src = (void *)sourcedata.data();
    int numinternalthreads = 1;

    int result = blosc_decompress_ctx(
        src,
        bloscArgs->decompressAddr,
        bloscArgs->decompressSize,
        numinternalthreads);
    if(result>0) return true;
    return false;
}

std::string CodecBlosc::getMessage() { return message;}

StructureConstPtr CodecBlosc::getCodecStructure()
{
    return CodecBlosc::codecStructure;
}

void CodecBlosc::initCodecStructure(const PVStructurePtr & pvStructure)
{
    PVStringArray::svector choices(4);
    choices[0] = "idle";
    choices[1] = "get";
    choices[2] = "put";
    choices[3] = "monitor";
    pvStructure->getSubField<PVStringArray>("command.choices")->replace(freeze(choices));
    choices.resize(6);
    choices[0] = "blosclz";
    choices[1] = "lz4";
    choices[2] = "lz4hc";
    choices[3] = "snappy";
    choices[4] = "zlib";
    choices[5] = "zstd";
    pvStructure->getSubField<PVStringArray>("bloscArgs.compressor.choices")->replace(freeze(choices));
    choices.resize(3);
    choices[0] = "NOSHUFFLE";
    choices[1] = "SHUFFLE";
    choices[2] = "BITSHUFFLE";
    pvStructure->getSubField<PVStringArray>("bloscArgs.shuffle.choices")->replace(freeze(choices));
    pvStructure->getSubField<PVInt>("bloscArgs.threads")->put(1);
    pvStructure->getSubField<PVInt>("bloscArgs.level")->put(3);
}

}}
