TOP=..

include $(TOP)/configure/CONFIG

scanServerPutGetSRC = $(TOP)/scanServerPutGet

EPICS_BASE_PVA_CORE_LIBS = pvDatabase pvAccess pvAccessCA pvData ca Com

INC += pv/scanServerPutGet.h

DBD += scanServerPutGetRegister.dbd

LIBRARY = scanServerPutGet
LIBSRCS += scanServerPutGet.cpp
LIBSRCS += scanServerPutGetRegister.cpp
scanServerPutGet_LIBS += scanService
scanServerPutGet_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

# shared library ABI version.
SHRLIB_VERSION ?= 4.3.0

PROD_HOST += scanServerPutGetMain
scanServerPutGetMain_SRCS += scanServerPutGetMain.cpp
scanServerPutGetMain_LIBS += scanServerPutGet
scanServerPutGetMain_LIBS += scanService
scanServerPutGetMain_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32


include $(TOP)/configure/RULES


