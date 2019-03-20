TOP=..

include $(TOP)/configure/CONFIG

scanServerRPCSRC = $(TOP)/scanServerRPC

EPICS_BASE_PVA_CORE_LIBS = pvDatabase pvAccess pvAccessCA pvData ca Com

INC += pv/scanServerRPC.h

DBD += scanServerRPCRegister.dbd

LIBRARY = scanServerRPC
LIBSRCS += scanServerRPC.cpp
LIBSRCS += scanServerRPCRegister.cpp
scanServerRPC_LIBS += scanService
scanServerRPC_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

# shared library ABI version.
SHRLIB_VERSION ?= 4.3.0

PROD_HOST += scanServerRPCMain
scanServerRPCMain_SRCS += scanServerRPCMain.cpp
scanServerRPCMain_LIBS += scanServerRPC
scanServerRPCMain_LIBS += scanService
scanServerRPCMain_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32


include $(TOP)/configure/RULES


