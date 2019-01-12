/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
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

class epicsShareClass CodecRecord :
    public epics::pvDatabase::PVRecord,
    public epicsThreadRunable
{
public:
    static CodecRecordPtr create(
        std::string const & recordName,
        std::string const & channelName,
        std::string const & codecName
        );
    virtual ~CodecRecord();
    virtual void process();
    virtual void run();

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
    std::auto_ptr<epicsThread> thread;
    epics::pvData::Event monitorEvent ;
    epics::pvData::Event runReturn;
};

}}

#endif  /* CODEC_RECORD_H */
