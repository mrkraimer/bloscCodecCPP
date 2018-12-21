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
  connected(false),
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

void CodecRecord::connect()
{
    string channelName(pvChannelName->get());
    PVDatabasePtr pvDatabase(PVDatabase::getMaster());
    PVRecordPtr pvRecord(pvDatabase->findRecord(channelName));
    if(pvRecord) {
         pvScalarArray = pvRecord->getPVStructure()->getSubField<PVScalarArray>("value");
         if(!pvScalarArray) {
              setAlarm(channelName + " no scalar array value",invalidAlarm,clientStatus);
              return;
         }
         connected = true;
         setAlarm(channelName + " is attached",noAlarm,clientStatus);
         return;
    }
    dbChannel *pchan = dbChannelCreate(channelName.c_str());
    if(pchan) {
cout << "found db record\n";
    } else {
cout << "did not find db record\n";
    }
    setAlarm(channelName + " is not attached",invalidAlarm,clientStatus);
    
}

void CodecRecord::disconnect()
{
   if(!connected) return;
   pvRecord.reset();
   pvScalarArray.reset();
   connected = false;
   setAlarm(pvChannelName->get() + " is idle",minorAlarm,clientStatus);
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
    switch(command) {
    case 0: // idle
          {
              disconnect();
          }
          break;
    case 1: // get
          {
              connect();
              if(!connected) break;
              string codecName = pvStructure->getSubField<PVString>("codecName")->get();
              if(codecName.compare("blosc")==0) {
                  bool result = codecBlosc->compressBlosc(
                      pvStructure->getSubField<PVUByteArray>("value"),
                      pvScalarArray,
                      pvStructure->getSubField<PVStructure>("bloscArgs")); 
                  return;
              }
              setAlarm("unknown codecName",invalidAlarm,clientStatus);
          }
          break;
    case 2: // put
          {
              connect();
              if(!connected) break;
              string codecName = pvStructure->getSubField<PVString>("codecName")->get();
              if(codecName.compare("blosc")==0) {
                  bool result = codecBlosc->decompressBlosc(
                      pvStructure->getSubField<PVUByteArray>("value"),
                      pvScalarArray,
                      pvStructure->getSubField<PVStructure>("bloscArgs")); 
                  return;
              }
              setAlarm("unknown codecName",invalidAlarm,clientStatus);
          }
          break;
    case 3: // monitor
          {
              setAlarm("monitor is not implemented",invalidAlarm,clientStatus);
          }
          break;
    default:
          {
              setAlarm("illegal command",invalidAlarm,clientStatus);
          }
    }
    PVRecord::process();
}

}}
