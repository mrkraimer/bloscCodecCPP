< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/bloscCodecIoc.dbd")
bloscCodecIoc_registerRecordDeviceDriver(pdbbase)

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
pvArrayCreateRecord PVRint8Array pvByte
pvArrayCreateRecord PVRint16Array pvShort
pvArrayCreateRecord PVRint32Array pvInt
pvArrayCreateRecord PVRint64Array pvLong
pvArrayCreateRecord PVRuint8Array pvUByte
pvArrayCreateRecord PVRuint16Array pvUShort
pvArrayCreateRecord PVRuint32Array pvUInt
pvArrayCreateRecord PVRuint64Array pvULong
pvArrayCreateRecord PVRfloatArray pvFloat
pvArrayCreateRecord PVRdoubleArray pvDouble

bloscCodecCreateRecord bloscCodecRecord


