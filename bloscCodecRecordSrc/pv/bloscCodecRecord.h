/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author Marty Kraimer
 * @date 2019.01
 */
#ifndef BLOSC_CODEC_RECORD_H
#define BLOSC_CODEC_RECORD_H

#include <epicsThread.h>
#include <pv/event.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/pvStructureCopy.h>

#include <pv/bloscCodec.h>

#include <shareLib.h>

namespace epics { namespace codec {

class BloscCodecRecord;
typedef std::tr1::shared_ptr<BloscCodecRecord> BloscCodecRecordPtr;
typedef std::tr1::weak_ptr<BloscCodecRecord> BloscCodecRecordWPtr;

/**
 * @brief A PVRecord that implements blosc compress/decompress.
 *
 *
 */
class epicsShareClass BloscCodecRecord :
    public epics::pvDatabase::PVRecord,
    public epicsThreadRunable
{
public:
    POINTER_DEFINITIONS(BloscCodecRecord);
    /**
     * @brief Factory method to create BloscCodecRecord.
     *
     * @param recordName The name for the BloscCodecRecord.
     * @param channelName Initial name of record to compress/decompress.
     * @param codecName The initial value for the name of the codec.
     * @return A shared pointer to BloscCodecRecord,
     */
    static BloscCodecRecordPtr create(
        std::string const & recordName,
        std::string const & channelName,
        std::string const & codecName
        );
    virtual ~BloscCodecRecord();
    /**
     * @brief perform requested command.
     */
    virtual void process();
    /**
     * @brief run method of thread that implements monitor.
     */
    virtual void run();
    /**
     * @brief p standard init method required by PVRecord
     *
     * @return true unless record name already exists.
     */
    bool init();
private:
    BloscCodecRecord(
        std::string const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
    void setAlarm(
        std::string const & message,
        epics::pvData::AlarmSeverity severity,
        epics::pvData::AlarmStatus status);
    bool compressPVRecord();
    bool decompressPVRecord();
    bool compressDBRecord();
    bool decompressDBRecord();
    void startMonitor();
    void stopMonitor();
    

    BloscCodecPtr bloscCodec;
    epics::pvData::PVStructurePtr pvStructure;
    epics::pvDatabase::PVRecordPtr pvRecord;
    epics::pvData::PVStringPtr pvChannelName;
    epics::pvData::PVUByteArrayPtr pvValue;
    epics::pvData::PVStructurePtr pvAlarmField;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::Alarm alarm;
    epics::pvData::PVStructurePtr pvTimeStampField;
    epics::pvData::PVTimeStamp pvTimeStamp;
    epics::pvData::TimeStamp timeStamp;

    bool monitorStarted;
    bool stopThread;
    std::string monitorChannelName;
    bool monitorIsPVRecord;
    std::auto_ptr<epicsThread> thread;
public:
    epics::pvData::Event monitorEvent ;
private:
    epics::pvData::Event runReturn;
};

}}

#endif  /* BLOSC_CODEC_RECORD_H */
