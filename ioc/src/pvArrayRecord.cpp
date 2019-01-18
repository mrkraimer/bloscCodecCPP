/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */

#include <pv/standardPVField.h>
#include <pv/ntscalar.h>
#include <pv/pvaClient.h>

#include <epicsExport.h>
#include <pv/pvArrayRecord.h>

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
using namespace epics::pvDatabase;
using namespace std;

namespace epics { namespace codec {

PVArrayRecordPtr PVArrayRecord::create(
    string const & recordName,
    string const & elementType)
{
    ScalarType scalarType;
    if(elementType.compare("pvByte")==0) {scalarType = pvByte;}
    else if(elementType.compare("pvShort")==0) {scalarType = pvShort;}
    else if(elementType.compare("pvInt")==0) {scalarType = pvInt;}
    else if(elementType.compare("pvLong")==0) {scalarType = pvLong;}
    else if(elementType.compare("pvFloat")==0) {scalarType = pvFloat;}
    else if(elementType.compare("pvDouble")==0) {scalarType = pvDouble;}
    else if(elementType.compare("pvUByte")==0) {scalarType = pvUByte;}
    else if(elementType.compare("pvUShort")==0) {scalarType = pvUShort;}
    else if(elementType.compare("pvUInt")==0) {scalarType = pvUInt;}
    else if(elementType.compare("pvULong")==0) {scalarType = pvULong;}
    else if(elementType.compare("pvString")==0) {scalarType = pvString;}
    else if(elementType.compare("pvBoolean")==0) {scalarType = pvBoolean;}
    else { cout << "illegal elementType\n"; return PVArrayRecordPtr();}
    PVStructurePtr pvStructure = getStandardPVField()->scalarArray(scalarType,"timeStamp");
    PVArrayRecordPtr pvRecord(
        new PVArrayRecord(
           recordName,pvStructure)); 
    if(!pvRecord->init()) pvRecord.reset();
    return pvRecord;
}

PVArrayRecord::PVArrayRecord(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure)
{
}


bool PVArrayRecord::init()
{
    initPVRecord();
    return true;
}


}}
