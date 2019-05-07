< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/scanServerPutGet.dbd")
scanServerPutGet_registerRecordDeviceDriver(pdbbase)

cd ${TOP}/iocBoot/${IOC}
iocInit()
scanServerPutGetCreateRecord scanServerPutGet
