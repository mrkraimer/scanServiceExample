< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/scanServerRPC.dbd")
scanServerRPC_registerRecordDeviceDriver(pdbbase)

cd ${TOP}/iocBoot/${IOC}
iocInit()
scanServerRPCCreateRecord scanServerRPC
