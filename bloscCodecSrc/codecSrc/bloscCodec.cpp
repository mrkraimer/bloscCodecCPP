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
#include <pv/pvIntrospect.h>

#include <shareLib.h>
#include <epicsExport.h>
#include <pv/bloscCodec.h>

namespace epics { namespace codec {

using namespace epics::pvData;
using namespace std;
using std::tr1::static_pointer_cast;

StructureConstPtr BloscCodec::codecStructure = BloscCodec::createCodecStructure();

StructureConstPtr BloscCodec::createCodecStructure()
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
    return bloscArgs;
}

BloscCodecPtr BloscCodec::create()
{
    BloscCodecPtr bloscCodec(new BloscCodec());
    return bloscCodec;
}
BloscCodec::BloscCodec(){}

bool BloscCodec::compressBlosc(
        const PVUByteArrayPtr & pvDest,
        const PVScalarArrayPtr & pvSource,
        const PVStructurePtr & pvBloscArgs)
{
    void * decompressAddr = NULL;
    size_t decompressSize = 0;
    int elementSize = 0;
    ScalarType scalarType = pvSource->getScalarArray()->getElementType();
    switch(scalarType)
    {
    case pvByte:
        {
             PVByteArrayPtr pvSrc = static_pointer_cast<PVByteArray>(pvSource);
             PVByteArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 1;
             decompressAddr = (void *)sourcedata.data();
        }
        break;
    case pvUByte:
        {
             PVUByteArrayPtr pvSrc = static_pointer_cast<PVUByteArray>(pvSource);
             PVUByteArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 1;
             decompressAddr = (void *)sourcedata.data();
        }
        break;
    case pvShort:
        {
             PVShortArrayPtr pvSrc = static_pointer_cast<PVShortArray>(pvSource);
             PVShortArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 2;
             decompressAddr = (void *)sourcedata.data();
        }
        break;
    case pvUShort:
        {
             PVUShortArrayPtr pvSrc = static_pointer_cast<PVUShortArray>(pvSource);
             PVUShortArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 2;
             decompressAddr = (void *)sourcedata.data();
             decompressSize = pvSource->getLength()*elementSize;
        }
        break;
    case pvInt:
        {
             PVIntArrayPtr pvSrc = static_pointer_cast<PVIntArray>(pvSource);
             PVIntArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 4;
             decompressAddr = (void *)sourcedata.data();
        }
        break;
    case pvUInt:
        {
             PVUIntArrayPtr pvSrc = static_pointer_cast<PVUIntArray>(pvSource);
             PVUIntArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 4;
             decompressAddr = (void *)sourcedata.data();
        }
        break;
    case pvLong:
        {
             PVLongArrayPtr pvSrc = static_pointer_cast<PVLongArray>(pvSource);
             PVLongArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 8;
             decompressAddr = (void *)sourcedata.data();
        }
        break;
    case pvULong:
        {
             PVULongArrayPtr pvSrc = static_pointer_cast<PVULongArray>(pvSource);
             PVULongArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 8;
             decompressAddr = (void *)sourcedata.data();
        }
        break;
    case pvFloat:
        {
             PVFloatArrayPtr pvSrc = static_pointer_cast<PVFloatArray>(pvSource);
             PVFloatArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 4;
             decompressAddr = (void *)sourcedata.data();
        }
        break;
    case pvDouble:
        {
             PVDoubleArrayPtr pvSrc = static_pointer_cast<PVDoubleArray>(pvSource);
             PVDoubleArray::const_svector sourcedata;
             pvSrc->getAs(sourcedata);
             elementSize = 8;
             decompressAddr = (void *)sourcedata.data(); 
        }
        break;
    default:
        string mess("BloscCodec::setBloscArgs pvType ");
        mess += scalarType;
        mess += " not supported";
        message = mess;
        return false;
    }
    decompressSize = pvSource->getLength()*elementSize;
    return compressBlosc(pvDest,decompressAddr,decompressSize,pvBloscArgs);
    
}

bool BloscCodec::compressBlosc(
        const PVUByteArrayPtr & pvDest,
        const void * decompressAddr, size_t decompressSize,
        const PVStructurePtr & pvBloscArgs)
{
    int clevel = pvBloscArgs->getSubField<PVInt>("level")->get();
    int doshuffle = pvBloscArgs->getSubField<PVInt>("shuffle.index")->get();
    size_t typesize = 1;
    size_t nbytes = decompressSize;
    size_t destsize = nbytes + BLOSC_MAX_OVERHEAD;
    PVUByteArray::svector destdata(destsize); 
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
        nbytes,decompressAddr,dest,
        destsize,compressor,
        blocksize,numinternalthreads);
    if(result>0) {
        destdata.resize(result);
        pvDest->replace(freeze(destdata));
        pvBloscArgs->getSubField<PVInt>("compressedSize")->put(result);
        pvBloscArgs->getSubField<PVInt>("decompressedSize")->put(decompressSize);
        return true;
    }
    return false;
}

bool BloscCodec::decompressBlosc(
    const epics::pvData::PVUByteArrayPtr & pvSource,
    const epics::pvData::PVScalarArrayPtr & pvDestination,
    const epics::pvData::PVStructurePtr & pvBloscArgs)
{
    size_t decompressSize = pvBloscArgs->getSubField<PVInt>("decompressedSize")->get();
    void * decompressAddr = NULL;
    ScalarType scalarType = pvDestination->getScalarArray()->getElementType();
    switch(scalarType)
    {
    case pvByte:
        {
             PVByteArrayPtr pvDest = static_pointer_cast<PVByteArray>(pvDestination);
             int elementsize = 1;
             size_t nelements = decompressSize/elementsize;
             PVByteArray::svector xxx(nelements);
             PVByteArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    case pvUByte:
        {
             PVUByteArrayPtr pvDest = static_pointer_cast<PVUByteArray>(pvDestination);
             int elementsize = 1;
             size_t nelements = decompressSize/elementsize;
             PVUByteArray::svector xxx(nelements);
             PVUByteArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    case pvShort:
        {
             PVShortArrayPtr pvDest = static_pointer_cast<PVShortArray>(pvDestination);
             int elementsize = 2;
             size_t nelements = decompressSize/elementsize;
             PVShortArray::svector xxx(nelements);
             PVShortArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    case pvUShort:
        {
             PVUShortArrayPtr pvDest = static_pointer_cast<PVUShortArray>(pvDestination);
             int elementsize = 2;
             size_t nelements = decompressSize/elementsize;
             PVUShortArray::svector xxx(nelements);
             PVUShortArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    case pvInt:
        {
             PVIntArrayPtr pvDest = static_pointer_cast<PVIntArray>(pvDestination);
             int elementsize = 4;
             size_t nelements = decompressSize/elementsize;
             PVIntArray::svector xxx(nelements);
             PVIntArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    case pvUInt:
        {
             PVUIntArrayPtr pvDest = static_pointer_cast<PVUIntArray>(pvDestination);
             int elementsize = 4;
             size_t nelements = decompressSize/elementsize;
             PVUIntArray::svector xxx(nelements);
             PVUIntArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
             break;
        }
        break;
    case pvLong:
        {
             PVLongArrayPtr pvDest = static_pointer_cast<PVLongArray>(pvDestination);
             int elementsize = 8;
             size_t nelements = decompressSize/elementsize;
             PVLongArray::svector xxx(nelements);
             PVLongArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    case pvULong:
        {
             PVULongArrayPtr pvDest = static_pointer_cast<PVULongArray>(pvDestination);
             int elementsize = 8;
             size_t nelements = decompressSize/elementsize;
             PVULongArray::svector xxx(nelements);
             PVULongArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    case pvFloat:
        {
             PVFloatArrayPtr pvDest = static_pointer_cast<PVFloatArray>(pvDestination);
             int elementsize = 4;
             size_t nelements = decompressSize/elementsize;
             PVFloatArray::svector xxx(nelements);
             PVFloatArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    case pvDouble:
        {
             PVDoubleArrayPtr pvDest = static_pointer_cast<PVDoubleArray>(pvDestination);
             int elementsize = 8;
             size_t nelements = decompressSize/elementsize;
             PVDoubleArray::svector xxx(nelements);
             PVDoubleArray::const_svector destdata(freeze(xxx));
             pvDest->replace(destdata);
             decompressAddr = (void *)destdata.data();
        }
        break;
    default:
        return false;
    }
    return decompressBlosc(pvSource,decompressAddr,decompressSize,pvBloscArgs);
}

bool BloscCodec::decompressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvSource,
        void * decompressAddr, size_t decompressSize,
        const epics::pvData::PVStructurePtr & pvBloscArgs)
{
    PVUByteArray::const_svector sourcedata;
    pvSource->getAs(sourcedata);   
    void *src = (void *)sourcedata.data();
    int numinternalthreads = 1;
    int result = blosc_decompress_ctx(
        src,
        decompressAddr,
        decompressSize,
        numinternalthreads);
    if(result<0) return false;
    return true;
}

std::string BloscCodec::getMessage() { return message;}

StructureConstPtr BloscCodec::getCodecStructure()
{
    return BloscCodec::codecStructure;
}

void BloscCodec::initCodecStructure(const PVStructurePtr & pvStructure)
{
    PVStringArray::svector choices(6);
    choices[0] = "blosclz";
    choices[1] = "lz4";
    choices[2] = "lz4hc";
    choices[3] = "snappy";
    choices[4] = "zlib";
    choices[5] = "zstd";
    pvStructure->getSubField<PVStringArray>("compressor.choices")->replace(freeze(choices));
    choices.resize(3);
    choices[0] = "NOSHUFFLE";
    choices[1] = "SHUFFLE";
    choices[2] = "BITSHUFFLE";
    pvStructure->getSubField<PVStringArray>("shuffle.choices")->replace(freeze(choices));
    pvStructure->getSubField<PVInt>("threads")->put(1);
    pvStructure->getSubField<PVInt>("level")->put(3);
}

}}
