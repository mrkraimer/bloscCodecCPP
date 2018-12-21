< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/codec.dbd")
codec_registerRecordDeviceDriver(pdbbase)

## Load record instance
dbLoadRecords("db/dbArray.db","name=DBRshortArray,nelem=500,type=DBF_SHORT");

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

