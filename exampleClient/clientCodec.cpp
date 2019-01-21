/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>
#include <sstream>
#include <epicsGetopt.h>
#include <pv/bloscCodec.h>


using namespace std;
using namespace epics::pvData;
using namespace epics::codec;


int main(int argc,char *argv[])
{
    PVDataCreatePtr pvDataCreate = getPVDataCreate();
    StructureConstPtr  topStructure = BloscCodec::getCodecStructure();
    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(topStructure);
    BloscCodecPtr bloscCodec(BloscCodec::create());   
    bloscCodec->initCodecStructure(pvStructure);
    cout << "pvStructure\n"  << pvStructure << "\n";
    return 0;
}
