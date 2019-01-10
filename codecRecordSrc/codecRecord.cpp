/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2013.08.02
 */

#include <dbAccess.h>
#include <dbChannel.h>
#include <dbStaticLib.h>
#include <dbNotify.h>
#include <special.h>
#include <pv/standardPVField.h>
#include <pv/ntscalar.h>
#include <pv/ntenum.h>

#include <pv/pvDatabase.h>
#include <pv/codecBlosc.h>

#include <epicsExport.h>
#include <pv/codecRecord.h>

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvDatabase;
using namespace epics::nt;
using std::tr1::static_pointer_cast;
using std::tr1::dynamic_pointer_cast;
using std::cout;
using std::endl;
using std::string;

namespace epics { namespace codec {
extern "C" {
typedef long (*get_array_info) (DBADDR *,long *,long *);
typedef long (*put_array_info) (void *,long);
}


CodecRecordPtr CodecRecord::create(
    string const & recordName,
    string const & channelName,
    string const & codecName)
{
    FieldCreatePtr fieldCreate = getFieldCreate();
    StandardFieldPtr standardField = getStandardField();
    PVDataCreatePtr pvDataCreate = getPVDataCreate();


    StructureConstPtr  topStructure = CodecBlosc::getCodecStructure();
    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(topStructure);
    pvStructure->getSubField<PVString>("channelName")->put(channelName);
   
    pvStructure->getSubField<PVString>("codecName")->put(codecName);
    CodecRecordPtr pvRecord(
        new CodecRecord(recordName,pvStructure)); 
    if(!pvRecord->init()) pvRecord.reset();
    return pvRecord;
}

CodecRecord::CodecRecord(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure),
  codecBlosc(CodecBlosc::create()),
  pvStructure(pvStructure)
{
}


bool CodecRecord::init()
{
    initPVRecord();
    codecBlosc->initCodecStructure(pvStructure);
    PVStructurePtr pvStructure = getPVRecordStructure()->getPVStructure();
    pvValue = pvStructure->getSubField<PVUByteArray>("value");
    if(!pvValue) {
        throw std::runtime_error("value is not a ubyte array");
    }
    pvAlarmField = pvStructure->getSubField<PVStructure>("alarm");
    if(!pvAlarmField) {
        throw std::runtime_error("no alarm field");
    }
    if(!pvAlarm.attach(pvAlarmField)) {
        throw std::runtime_error(string("bad alarm field"));
    }
    pvTimeStampField = pvStructure->getSubField<PVStructure>("timeStamp");
    if(!pvTimeStamp.attach(pvTimeStampField)) {
        throw std::runtime_error(string("bad timeStamp field"));
    }
    pvChannelName = pvStructure->getSubField<PVString>("channelName");
    if(!pvChannelName) {
        throw std::runtime_error("channelName is not a string");
    }
    setAlarm(pvChannelName->get() + " is idle",minorAlarm,clientStatus);
    return true;
}

bool CodecRecord::compressPVRecord()
{
    string channelName(pvChannelName->get());
    PVDatabasePtr pvDatabase(PVDatabase::getMaster());
    PVRecordPtr pvRecord(pvDatabase->findRecord(channelName));
    if(!pvRecord) return false;
    PVScalarArrayPtr pvScalarArray = 
              pvRecord->getPVStructure()->getSubField<PVScalarArray>("value");
    if(!pvScalarArray) {
         setAlarm(channelName + " no scalar array value",invalidAlarm,clientStatus);
        return false;
    }
    ScalarType scalarType(pvScalarArray->getScalarArray()->getElementType());
    pvStructure->getSubField<PVInt>("elementScalarType")->put(scalarType);
    string codecName = pvStructure->getSubField<PVString>("codecName")->get();
    if(codecName.compare("blosc")!=0) {
        setAlarm(codecName + " codec not supported",invalidAlarm,clientStatus);
        return false;
    }
    pvRecord->lock();
    try {
        bool result = codecBlosc->compressBlosc(
            pvStructure->getSubField<PVUByteArray>("value"),
            pvScalarArray,
            pvStructure->getSubField<PVStructure>("bloscArgs")); 
        if(result) {
            setAlarm("compressBlosc success",noAlarm,clientStatus);
        } else {
            setAlarm("compressBlosc failure",invalidAlarm,clientStatus);
        }
    } catch(...) {
         setAlarm("compressBlosc exception",invalidAlarm,clientStatus);
    }
    pvRecord->unlock();
    return true;
}

bool CodecRecord::decompressPVRecord()
{
    string channelName(pvChannelName->get());
    PVDatabasePtr pvDatabase(PVDatabase::getMaster());
    PVRecordPtr pvRecord(pvDatabase->findRecord(channelName));
    if(!pvRecord) return false;
    PVScalarArrayPtr pvScalarArray = 
              pvRecord->getPVStructure()->getSubField<PVScalarArray>("value");
    if(!pvScalarArray) {
         setAlarm(channelName + " no scalar array value",invalidAlarm,clientStatus);
        return false;
    }
    string codecName = pvStructure->getSubField<PVString>("codecName")->get();
    if(codecName.compare("blosc")!=0) {
        setAlarm(codecName + " codec not supported",invalidAlarm,clientStatus);
        return false;
    }
    pvRecord->lock();
    try {
        bool result = codecBlosc->decompressBlosc(
            pvStructure->getSubField<PVUByteArray>("value"),
            pvScalarArray,
            pvStructure->getSubField<PVStructure>("bloscArgs")); 
        if(result) {
            setAlarm("decompressBlosc success",noAlarm,clientStatus);
        } else {
            setAlarm("decompressBlosc failure",invalidAlarm,clientStatus);
        }
    } catch(...) {
         setAlarm("decompressBlosc exception",invalidAlarm,clientStatus);
    }
    pvRecord->unlock();
    return true;
}


bool CodecRecord::compressDBRecord()
{
    string channelName(pvChannelName->get());
    dbChannel *pchan = dbChannelCreate(channelName.c_str());
    if(pchan==NULL) return false;
    long status = dbChannelOpen(pchan);
    if (status) {
        setAlarm(channelName + " dbChannelOpen failed",invalidAlarm,clientStatus);
        return true;
    }
    Type type = (dbChannelSpecial(pchan)==SPC_DBADDR) ? scalarArray : scalar;
    if(type!=scalarArray) { 
        setAlarm(channelName + " is not a scalarArray",invalidAlarm,clientStatus);
        return true;
    }
    int elementsize = 0;
    ScalarType scalarType(pvByte);
    pvStructure->getSubField<PVInt>("elementScalarType")->put(scalarType);
    switch (dbChannelFieldType(pchan)) {
    case DBF_CHAR:
        elementsize = 1; scalarType = pvByte; break;
    case DBF_UCHAR:
        elementsize = 1; scalarType = pvUByte; break;
    case DBF_SHORT:
        elementsize = 2; scalarType = pvShort; break;
    case DBF_USHORT:
        elementsize = 2; scalarType = pvUShort; break;
    case DBF_LONG:
        elementsize = 4; scalarType = pvInt; break;
    case DBF_ULONG:
        elementsize = 4; scalarType = pvUInt; break;
    case DBF_INT64:
        elementsize = 1; scalarType = pvLong; break;
    case DBF_UINT64:
        elementsize = 4; scalarType = pvULong; break;
    case DBF_FLOAT:
        elementsize = 4; scalarType = pvFloat; break;
    case DBF_DOUBLE:
        elementsize = 8; scalarType = pvDouble; break;
    default:
        setAlarm(channelName + " unsupported DBF_type",invalidAlarm,clientStatus);
        return true;
    }
    pvStructure->getSubField<PVInt>("elementScalarType")->put(scalarType);
    struct dbCommon *precord = dbChannelRecord(pchan);
    dbScanLock(precord);
    long rec_length = 0;
    long rec_offset = 0;
    rset *prset = dbGetRset(&pchan->addr);
    get_array_info get_info;
    get_info = (get_array_info)(prset->get_array_info);
    get_info(&pchan->addr, &rec_length, &rec_offset);
    if(rec_offset!=0) {
        setAlarm(" Can't handle offset != 0",invalidAlarm,clientStatus);
        dbScanUnlock(precord);
        return false;
    }
    const void * decompressAddr = dbChannelField(pchan);
    size_t length = rec_length;
    size_t decompressSize = length * elementsize;
    string codecName = pvStructure->getSubField<PVString>("codecName")->get();
    if(codecName.compare("blosc")!=0) {
        setAlarm(codecName + " codec not supported",invalidAlarm,clientStatus);
        dbScanUnlock(precord);
        return false;
    }
    bool result = codecBlosc->compressBlosc(
        pvStructure->getSubField<PVUByteArray>("value"),
        decompressAddr,decompressSize,
        pvStructure->getSubField<PVStructure>("bloscArgs")); 
    if(result) {
        setAlarm("compressBlosc success",noAlarm,clientStatus);
    } else {
        setAlarm("compressBlosc failure",invalidAlarm,clientStatus);
    }
    dbScanUnlock(precord);
    return true;
}

bool CodecRecord::decompressDBRecord()
{
    string channelName(pvChannelName->get());
    dbChannel *pchan = dbChannelCreate(channelName.c_str());
    if(pchan==NULL) return false;
    long status = dbChannelOpen(pchan);
    if (status) {
        setAlarm(channelName + " dbChannelOpen failed",invalidAlarm,clientStatus);
        return true;
    }
    Type type = (dbChannelSpecial(pchan)==SPC_DBADDR) ? scalarArray : scalar;
    if(type!=scalarArray) {
        setAlarm(channelName + " is not a scalarArray",invalidAlarm,clientStatus);
        return true;
    }
    long rec_length = 0;
    long rec_offset = 0;
    rset *prset = dbGetRset(&pchan->addr);
    get_array_info get_info;
    get_info = (get_array_info)(prset->get_array_info);
    get_info(&pchan->addr, &rec_length, &rec_offset);
    if(rec_offset!=0) {
           throw std::logic_error("Can't handle offset != 0");
    }
    void * decompressAddr = dbChannelField(pchan);
    int elementsize = 0;
    switch (dbChannelFieldType(pchan)) {
    case DBF_CHAR:
    case DBF_UCHAR:
        elementsize = 1; break;
    case DBF_SHORT:
    case DBF_USHORT:
        elementsize = 2; break;
    case DBF_LONG:
    case DBF_ULONG:
        elementsize = 4; break;
    case DBF_INT64:
    case DBF_UINT64:
        elementsize = 4; break;
    case DBF_FLOAT:
        elementsize = 4; break;
    case DBF_DOUBLE:
        elementsize = 8; break;
    default:
        setAlarm(channelName + " unsupported DBF_type",invalidAlarm,clientStatus);
        return true;
    }
    size_t decompressSize = pvStructure->getSubField<PVInt>("bloscArgs.decompressedSize")->get();
    long max_elements  = dbChannelFinalElements(pchan);
    size_t maxbytes = max_elements*elementsize;
    if(decompressSize>maxbytes) decompressSize = maxbytes;
    string codecName = pvStructure->getSubField<PVString>("codecName")->get();
    if(codecName.compare("blosc")!=0) {
        setAlarm(codecName + " codec not supported",invalidAlarm,clientStatus);
        return false;
    }
    struct dbCommon *precord = dbChannelRecord(pchan);
    dbScanLock(precord);
    bool result = codecBlosc->decompressBlosc(
        pvStructure->getSubField<PVUByteArray>("value"),
        decompressAddr,decompressSize,
        pvStructure->getSubField<PVStructure>("bloscArgs")); 
    if(result) {
        setAlarm("decompressBlosc success",noAlarm,clientStatus);
        long nelements = decompressSize/elementsize;
        put_array_info put_info;
        put_info = (put_array_info)(prset->put_array_info);
        put_info(&pchan->addr, nelements);
        dbProcess(precord);
    } else {
        setAlarm("decompressBlosc failure",invalidAlarm,clientStatus);
    }
    dbScanUnlock(precord);
    return true;
}

void CodecRecord::setAlarm(string const & message,AlarmSeverity severity,AlarmStatus status)
{
    alarm.setMessage(message);
    alarm.setSeverity(severity);
    alarm.setStatus(status);
    pvAlarm.set(alarm);
}

void CodecRecord::process()
{
    int command = pvStructure->getSubField<PVInt>("command.index")->get();
    string channelName(pvChannelName->get());
    switch(command) {
    case 0: // idle
          break;
    case 1: // get
          if(compressPVRecord()) break;
          if(compressDBRecord()) break;
          setAlarm(channelName +" does not exist",noAlarm,clientStatus);
          break;
    case 2: // put
          if(decompressPVRecord()) break;
          if(decompressDBRecord()) break;
          setAlarm(channelName +" does not exist",noAlarm,clientStatus);
          break;
    case 3: // monitor
          setAlarm("monitor is not implemented",invalidAlarm,clientStatus);
          break;
    default:
          setAlarm("illegal command",invalidAlarm,clientStatus);
    }
    PVRecord::process();
}

}}
