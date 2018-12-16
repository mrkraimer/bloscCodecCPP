/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2013.08.02
 */

#include <pv/standardPVField.h>
#include <pv/ntscalar.h>
#include <pv/ntenum.h>
#include <pv/pvaClient.h>

#include <epicsExport.h>
#include <pv/codecRecord.h>

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
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

    StructureConstPtr  topStructure = fieldCreate->createFieldBuilder()->
         addArray("value",pvUByte)->
         addNestedStructure("codec")->
             add("name",pvString)->
             add("parameters",fieldCreate->createVariantUnion())->
             endNested()->
         add("alarm",standardField->alarm()) ->
         add("timeStamp",standardField->timeStamp()) -> 
         add("channelName",pvString)->
         add("channelType",pvString)->
         add("compressedSize",pvLong)->
         add("decompressedSize",pvLong)->
         add("mode",standardField->enumerated()) ->
         createStructure();
    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(topStructure);
    pvStructure->getSubField<PVString>("channelName")->put(channelName);
    PVStringArray::svector modechoices(2);
    modechoices[0] = "compress";
    modechoices[1] = "decompress";
    pvStructure->getSubField<PVStringArray>("mode.choices")->replace(freeze(modechoices));
    pvStructure->getSubField<PVString>("codec.name")->put(codecName);
    if(codecName.compare("blosc")==0) {
        PVUnionPtr pvParameters = pvStructure->getSubField<PVUnion>("codec.parameters");
        NTEnumBuilderPtr enumBuilder = NTEnum::createBuilder();
        PVStructurePtr pvEnum = enumBuilder->
            createPVStructure();
        PVStringArray::svector choices(6);
        choices[0] = "BloscLZ";
        choices[1] = "LZ4";
        choices[2] = "LZ4HC";
        choices[3] = "SNAPPY";
        choices[4] = "ZLIB";
        choices[5] = "ZSTD";
        pvEnum->getSubField<PVStringArray>("value.choices")->replace(freeze(choices));
        pvParameters->set(pvEnum);
    }
    CodecRecordPtr pvRecord(
        new CodecRecord(recordName,pvStructure)); 
    if(!pvRecord->init()) pvRecord.reset();
    return pvRecord;
}

CodecRecord::CodecRecord(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure)
{
}


bool CodecRecord::init()
{
    initPVRecord();
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
    PVStringPtr pvString = pvStructure->getSubField<PVString>("channelName");
    if(!pvString) {
        throw std::runtime_error("channelName is not a string");
    }
    alarm.setMessage(pvString->get() + " is not attached");
    alarm.setSeverity(invalidAlarm);
    alarm.setStatus(clientStatus);
    pvAlarm.set(alarm);
    return true;
}

void CodecRecord::process()
{
    PVRecord::process();
}

}}
