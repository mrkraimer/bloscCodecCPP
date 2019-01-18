/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */
#ifndef PVARRAYRECORD_H
#define PVARRAYRECORD_H

#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/pvaClient.h>

#include <shareLib.h>

namespace epics { namespace codec {


class PVArrayRecord;
typedef std::tr1::shared_ptr<PVArrayRecord> PVArrayRecordPtr;
typedef std::tr1::weak_ptr<PVArrayRecord> PVArrayRecordWPtr;


class epicsShareClass PVArrayRecord :
    public epics::pvDatabase::PVRecord
{
public:
    POINTER_DEFINITIONS(PVArrayRecord);
    static PVArrayRecordPtr create(
        std::string const & recordName,
        std::string const & elementType
        );
    virtual ~PVArrayRecord() {}
    virtual bool init();
private:
    PVArrayRecord(
        std::string const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
};

}}

#endif  /* PVARRAYRECORD_H */
