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
#include <dbEvent.h>
#include <dbStaticLib.h>
#include <special.h>
#include <epicsThread.h>
#include <pv/event.h>
#include <pv/timeStamp.h>
#include <pv/standardPVField.h>
#include <pv/ntscalar.h>
#include <pv/ntenum.h>
#include <pv/pvDatabase.h>
#include <pv/pvStructureCopy.h>
#include <pv/bloscCodec.h>

#include <epicsExport.h>
#include <pv/bloscCodecRecord.h>

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvDatabase;
using namespace epics::pvCopy;
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


BloscCodecRecordPtr BloscCodecRecord::create(string const & recordName)
{
    FieldCreatePtr fieldCreate = getFieldCreate();
    StandardFieldPtr standardField = getStandardField();
    PVDataCreatePtr pvDataCreate = getPVDataCreate();

    StructureConstPtr bloscArgs(BloscCodec::getCodecStructure());
    StructureConstPtr  topStructure = fieldCreate->createFieldBuilder()->
         addArray("value",pvUByte)->
         add("alarm",standardField->alarm()) ->
         add("timeStamp",standardField->timeStamp()) -> 
         add("channelName",pvString)->
         add("elementScalarType",pvInt)->
         add("command",standardField->enumerated()) ->
         add("bloscArgs",bloscArgs) ->
         createStructure();
    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(topStructure);
    PVStringArray::svector choices(5);
    choices[0] = "idle";
    choices[1] = "get";
    choices[2] = "put";
    choices[3] = "startMonitor";
    choices[4] = "stopMonitor";
    pvStructure->getSubField<PVStringArray>("command.choices")->replace(freeze(choices));
    BloscCodecRecordPtr pvRecord(
        new BloscCodecRecord(recordName,pvStructure)); 
    if(!pvRecord->init()) pvRecord.reset();
    return pvRecord;
}

BloscCodecRecord::BloscCodecRecord(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure),
  bloscCodec(BloscCodec::create()),
  pvStructure(pvStructure),
  monitorStarted(false),
  monitorIsPVRecord(false),
  stopThread(false),
  monitorLevel(0),
  monitorCompressorIndex(0),
  monitorShuffleIndex(0),
  monitorThreads(1)
{
}

BloscCodecRecord::~BloscCodecRecord()
{
    if(!monitorStarted) return;
    stopThread = true;
    monitorEvent.signal();
    runReturn.wait();
}

bool BloscCodecRecord::init()
{
    initPVRecord();
    bloscCodec->initCodecStructure(pvStructure->getSubField<PVStructure>("bloscArgs"));
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

bool BloscCodecRecord::compressPVRecord()
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
    pvRecord->lock();
    try {
        bool result = bloscCodec->compressBlosc(
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

bool BloscCodecRecord::decompressPVRecord()
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
    pvRecord->lock();
    try {
        bool result = bloscCodec->decompressBlosc(
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


bool BloscCodecRecord::compressDBRecord()
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
        elementsize = 8; scalarType = pvLong; break;
    case DBF_UINT64:
        elementsize = 8; scalarType = pvULong; break;
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
    bool result = bloscCodec->compressBlosc(
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

bool BloscCodecRecord::decompressDBRecord()
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
        elementsize = 8; break;
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
    struct dbCommon *precord = dbChannelRecord(pchan);
    dbScanLock(precord);
    bool result = bloscCodec->decompressBlosc(
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

void BloscCodecRecord::setAlarm(string const & message,AlarmSeverity severity,AlarmStatus status)
{
    alarm.setMessage(message);
    alarm.setSeverity(severity);
    alarm.setStatus(status);
    pvAlarm.set(alarm);
}

void BloscCodecRecord::process()
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
    case 3: // startMonitor
          startMonitor();
          break;
    case 4: // stopMonitor
          stopMonitor();
          break;
    default:
          setAlarm("illegal command",invalidAlarm,clientStatus);
    }
    PVRecord::process();
}

class MyListener;
typedef std::tr1::shared_ptr<MyListener> MyListenerPtr;
typedef std::tr1::weak_ptr<MyListener> MyListenerWPtr;

class MyListener :
    public PVListener,
    public std::tr1::enable_shared_from_this<MyListener>
{
public:
    POINTER_DEFINITIONS(MyListener);
    virtual void detach(PVRecordPtr const & pvRecord) {}
    virtual void unlisten(PVRecordPtr const & pvRecord) {}
    virtual void dataPut(PVRecordFieldPtr const & pvRecordField){}
    virtual void dataPut(
        PVRecordStructurePtr const & requested,
        PVRecordFieldPtr const & pvRecordField) {}
    virtual void beginGroupPut(PVRecordPtr const & pvRecord) {}
    virtual void endGroupPut(PVRecordPtr const & pvRecord);
    static MyListenerPtr create(Event * event);
private:
    Event* monitorEvent;
    MyListener(Event * event)
    : monitorEvent(event)
    {}
};

MyListenerPtr MyListener::create(Event * event)
{
    MyListenerPtr myListener(new MyListener(event));
    return myListener;
}

void MyListener::endGroupPut(PVRecordPtr const & pvRecord)
{
     monitorEvent->signal();
}


static void pdb_single_event(void *user_arg, struct dbChannel *chan,
                      int eventsRemaining, struct db_field_log *pfl)
{
    BloscCodecRecord *bloscCodecRecord = static_cast<BloscCodecRecord *>(user_arg);
    bloscCodecRecord->monitorEvent.signal();
}


void BloscCodecRecord::run()
{
   static bool firstTime(true);
   static dbEventCtx event_context(NULL);
   if(firstTime) {
        firstTime = false;
        event_context = db_init_events();
        if(!event_context) {
            throw std::runtime_error("Failed to create dbEvent context");
        }
        int ret = db_start_events(event_context,
            "bloscCodecRecordMonitor", NULL, NULL, epicsThreadPriorityCAServerLow);
        if(ret!=DB_EVENT_OK) {
            throw std::runtime_error("Failed to start dbEvent context");
        }
   }
   PVRecordPtr pvRecord;
   PVCopyPtr pvCopy;
   MyListenerPtr listener;
   dbChannel *chan = NULL;
   dbEventCtx eventContext = NULL;
   dbEventSubscription subscript = NULL;
   if(monitorIsPVRecord) {
       PVDatabasePtr pvDatabase(PVDatabase::getMaster());
       pvRecord = pvDatabase->findRecord(monitorChannelName);
       PVStructurePtr pvTop = pvRecord->getPVRecordStructure()->getPVStructure();
       string request("field(value[array=0:1])");
       PVStructurePtr pvRequest(CreateRequest::create()->createRequest(request));
       pvCopy = PVCopy::create(pvTop,pvRequest,"");
       listener = MyListener::create(&monitorEvent);
       pvRecord->addListener(listener,pvCopy);
   } else {
       chan = dbChannelCreate(monitorChannelName.c_str());
       if(!chan) {
           throw std::runtime_error(monitorChannelName + " not found");
       }
       eventContext = db_init_events();
       subscript = db_add_event(
            eventContext,chan,&pdb_single_event, this, DBE_VALUE
);
       if(!subscript) {
           throw std::runtime_error(monitorChannelName +" db_add_event failed");
       }
       int ret = db_start_events(
           eventContext, "bloscCodecRecordMonitor", NULL, NULL,
           epicsThreadPriorityCAServerLow);
       db_post_single_event(subscript);
       db_event_enable(subscript);
   }
   while(true) {
       monitorEvent.wait();
       if(stopThread) {
           runReturn.signal();
           return;
       }
       lock();
       try {
           beginGroupPut();
           pvStructure->getSubField<PVInt>("command.index")->put(1);
           pvStructure->getSubField<PVString>("channelName")->put(monitorChannelName);
           pvStructure->getSubField<PVInt>("bloscArgs.level")->put(monitorLevel);
           pvStructure->getSubField<PVInt>("bloscArgs.compressor.index")->put(monitorCompressorIndex);
           pvStructure->getSubField<PVInt>("bloscArgs.shuffle.index")->put(monitorShuffleIndex);
           pvStructure->getSubField<PVInt>("bloscArgs.threads")->put(monitorThreads);
           process();
           endGroupPut();
        } catch(...) {
             setAlarm("compressBlosc exception",invalidAlarm,clientStatus);
        }
        unlock();
   }
   if(monitorIsPVRecord) {
       pvRecord->removeListener(listener,pvCopy);
   } else {
       db_cancel_event(subscript);
   }
   runReturn.signal();
}

void BloscCodecRecord::startMonitor()
{
    if(monitorStarted) {
       setAlarm("monitor is already started",noAlarm,clientStatus);
       return;
    }
    string channelName(pvChannelName->get());
    PVDatabasePtr pvDatabase(PVDatabase::getMaster());
    PVRecordPtr pvRecord(pvDatabase->findRecord(channelName));
    if(pvRecord) {
        monitorIsPVRecord = true;
    } else {
       dbChannel *pchan = dbChannelCreate(channelName.c_str());
       if(pchan==NULL) {
           setAlarm(channelName +" is not found",noAlarm,clientStatus);
           return;
       }
       monitorIsPVRecord = false;
    }
    monitorChannelName = channelName;
    monitorLevel = pvStructure->getSubField<PVInt>("bloscArgs.level")->get();
    monitorCompressorIndex = pvStructure->getSubField<PVInt>("bloscArgs.compressor.index")->get();
    monitorShuffleIndex = pvStructure->getSubField<PVInt>("bloscArgs.shuffle.index")->get();
    monitorThreads = pvStructure->getSubField<PVInt>("bloscArgs.threads")->get();

    monitorStarted = true;
    thread =  std::auto_ptr<epicsThread>(new epicsThread(
        *this,
        "bloscCodecRecord",
        epicsThreadGetStackSize(epicsThreadStackSmall),
        epicsThreadPriorityLow));
    thread->start();
}

void BloscCodecRecord::stopMonitor()
{
    stopThread = true;
    monitorEvent.signal();
    runReturn.wait();
    stopThread = false;
    monitorStarted = false;
}

}}
