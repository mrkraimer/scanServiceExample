TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library
#

DBD += scanServerPutGet.dbd

#=============================
# build an ioc application

PROD_IOC += scanServerPutGet


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
scanServerPutGet_SRCS += scanServerPutGet_registerRecordDeviceDriver.cpp
scanServerPutGet_SRCS_DEFAULT += scanServerPutGetMain.cpp
scanServerPutGet_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
scanServerPutGet_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

scanServerPutGet_LIBS += scanServerPutGet
scanServerPutGet_LIBS += pvDatabase qsrv pvAccessIOC pvAccess pvAccessCA nt pvData
scanServerPutGet_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

