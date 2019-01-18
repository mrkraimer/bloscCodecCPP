/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */


/* Author: Marty Kraimer */

#include <cstddef>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <cstdio>
#include <memory>
#include <iostream>

#include <cantProceed.h>
#include <epicsStdio.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsThread.h>
#include <iocsh.h>

#include <pv/pvIntrospect.h>
#include <pv/pvData.h>
#include <pv/standardField.h>
#include <pv/standardPVField.h>
#include <pv/pvAccess.h>
#include <pv/ntscalarArray.h>
#include <pv/pvaClient.h>
#include <pv/pvDatabase.h>

#include <epicsExport.h>
#include <pv/pvArrayRecord.h>

using namespace epics::pvData;
using namespace epics::nt;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
using namespace epics::pvDatabase;
using namespace epics::codec;
using std::cout;
using std::endl;
using std::string;

static StandardPVFieldPtr standardPVField = getStandardPVField();

static const iocshArg testArg0 = { "recordName", iocshArgString };
static const iocshArg testArg1 = { "elementType", iocshArgString };

static const iocshArg *testArgs[] = {
    &testArg0,&testArg1};

static const iocshFuncDef pvArrayFuncDef = {
    "pvArrayCreateRecord", 2, testArgs};
static void pvArrayCallFunc(const iocshArgBuf *args)
{
    string recordName("PVRintArray");
    string elementType("pvInt");
    char *sval = args[0].sval;
    if(sval) recordName = string(sval);
    sval = args[1].sval;
    if(sval) elementType = string(sval);
    PVDatabasePtr master = PVDatabase::getMaster();
    PVArrayRecordPtr record(PVArrayRecord::create(recordName,elementType));
    bool result(false);
    if(record) 
        result = master->addRecord(record);
    if(!result) cout << "recordname" << " not added" << endl;
    
}

static void pvArrayRegister(void)
{
    static int firstTime = 1;
    if (firstTime) {
        firstTime = 0;
        iocshRegister(&pvArrayFuncDef, pvArrayCallFunc);
    }
}

extern "C" {
    epicsExportRegistrar(pvArrayRegister);
} 
