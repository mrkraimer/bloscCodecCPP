/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author Marty Kraimer
 * @date 2019.01
 */
#ifndef CODEC_RECORD_H
#define CODEC_RECORD_H

#include <epicsThread.h>
#include <pv/event.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/pvStructureCopy.h>

#include <pv/codecBlosc.h>

#include <shareLib.h>

namespace epics { namespace codec {

class CodecRecord;
typedef std::tr1::shared_ptr<CodecRecord> CodecRecordPtr;
typedef std::tr1::weak_ptr<CodecRecord> CodecRecordWPtr;

/**
 * @brief A PVRecord that implements blosc compress/decompress.
 *
 *
 */
class epicsShareClass CodecRecord :
    public epics::pvDatabase::PVRecord,
    public epicsThreadRunable
{
public:
    POINTER_DEFINITIONS(CodecRecord);
    /**
     * @brief Factory method to create CodecRecord.
     *
     * @param recordName The name for the CodecRecord.
     * @param channelName Initial name of record to compress/decompress.
     * @param codecName The initial value for the name of the codec.
     * @return A shared pointer to CodecRecord,
     */
    static CodecRecordPtr create(
        std::string const & recordName,
        std::string const & channelName,
        std::string const & codecName
        );
    virtual ~CodecRecord();
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
    CodecRecord(
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
    

    CodecBloscPtr codecBlosc;
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

#endif  /* CODEC_RECORD_H */
