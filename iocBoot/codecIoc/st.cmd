< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/codecIoc.dbd")
codecIoc_registerRecordDeviceDriver(pdbbase)

## Load record instance
dbLoadRecords("db/dbArray.db","name=DBRint8Array,nelem=500,type=CHAR");
dbLoadRecords("db/dbArray.db","name=DBRint16Array,nelem=500,type=SHORT");
dbLoadRecords("db/dbArray.db","name=DBRint32Array,nelem=500,type=LONG");
dbLoadRecords("db/dbArray.db","name=DBRint64Array,nelem=500,type=INT64");
dbLoadRecords("db/dbArray.db","name=DBRuint8Array,nelem=500,type=UCHAR");
dbLoadRecords("db/dbArray.db","name=DBRuint16Array,nelem=500,type=USHORT");
dbLoadRecords("db/dbArray.db","name=DBRuint32Array,nelem=500,type=ULONG");
dbLoadRecords("db/dbArray.db","name=DBRuint64Array,nelem=500,type=UINT64");
dbLoadRecords("db/dbArray.db","name=DBRfloatArray,nelem=500,type=FLOAT");
dbLoadRecords("db/dbArray.db","name=DBRdoubleArray,nelem=500,type=DOUBLE");



cd ${TOP}/iocBoot/${IOC}
iocInit()
pvArrayCreateRecord PVRbyteArray pvByte
pvArrayCreateRecord PVRshortArray pvShort
pvArrayCreateRecord PVRintArray pvInt
pvArrayCreateRecord PVRlongArray pvLong
pvArrayCreateRecord PVRfloatArray pvFloat
pvArrayCreateRecord PVRdoubleArray pvDouble
pvArrayCreateRecord PVRstringArray pvString
pvArrayCreateRecord PVRbooleanArray pvBoolean
pvArrayCreateRecord PVRubyteArray pvUByte
pvArrayCreateRecord PVRushortArray pvUShort
pvArrayCreateRecord PVRuintArray pvUInt
pvArrayCreateRecord PVRulongArray pvULong
codecCreateRecord codecRecord PVRubyteArray blosc

