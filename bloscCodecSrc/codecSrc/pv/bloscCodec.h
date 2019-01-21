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
/**
 * @brief Factory method to create a BloscCodec.
 *
 * @return the BloscCodec.
 */
    static BloscCodecPtr create();
    virtual ~BloscCodec() {}

private:
    BloscCodec();
    std::string message;
    static epics::pvData::StructureConstPtr codecStructure;
    static epics::pvData::StructureConstPtr createCodecStructure();
public:
/**
 * @brief return the introspection interface for a BloscCodec.
 *
 * @return The introspection structure.
 */
    static epics::pvData::StructureConstPtr getCodecStructure();
/**
 * @brief compress a pvStructure
 *
 * @param pvDest The location of the compressed data.
 * @param pvSource The pvStructure to compress.
 * @param pvBlosccArgs The blosc details.
 * @return (true,false) means (success,failure)
 * On failure getMessage returns a reason.
 */
    bool compressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvDest,
        const epics::pvData::PVScalarArrayPtr & pvSource,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
/**
 * @brief compress given an address and sise.
 *
 * @param pvDest The location of the compressed data.
 * @param decompressAddr The address of the array to compress.
 * @param decompressSize The number of bytes to compress.
 * @param pvBlosccArgs The blosc details.
 * @return (true,false) means (success,failure)
 * On failure getMessage returns a reason.
 */
    bool compressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvDest,
        const void * decompressAddr, size_t decompressSize,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
/**
 * @brief decompress into a pvStructure
 *
 * @param pvSource The location of the compressed data.
 * @param pvDest The pvStructure for the uncompressed data.
 * @param pvBlosccArgs The blosc details.
 * @return (true,false) means (success,failure)
 * On failure getMessage returns a reason.
 */
    bool decompressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvSource,
        const epics::pvData::PVScalarArrayPtr & pvDest,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
/**
 * @brief decompress given an address and sise.
 *
 * @param pvSource The location of the compressed data.
 * @param decompressAddr The address of the array to compress.
 * @param decompressSize The number of bytes to compress.
 * @param pvBlosccArgs The blosc details.
 * @return (true,false) means (success,failure)
 * On failure getMessage returns a reason.
 */
   bool decompressBlosc(
        const epics::pvData::PVUByteArrayPtr & pvSource,
        void * decompressAddr, size_t decompressSize,
        const epics::pvData::PVStructurePtr & pvBloscArgs);
/**
 * @brief Get a message if compress or decompress fails.
 *
 * @return The reason for a failure.
 */
    std::string getMessage();
/**
 * @brief Initialaze a BloscCodec pvStructure.
 *
 * @param pvStructure The BloscCodec pvStructure.
 */
    void initCodecStructure(const epics::pvData::PVStructurePtr & pvStructure);
};

}}

#endif  /* BLOSC_CODEC_H */
