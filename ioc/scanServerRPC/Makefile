TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library
#

DBD += scanServerRPC.dbd

#=============================
# build an ioc application

PROD_IOC += scanServerRPC


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
scanServerRPC_SRCS += scanServerRPC_registerRecordDeviceDriver.cpp
scanServerRPC_SRCS_DEFAULT += scanServerRPCMain.cpp
scanServerRPC_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
scanServerRPC_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

scanServerRPC_LIBS += scanServerRPC
scanServerRPC_LIBS += pvDatabase qsrv pvAccessIOC pvAccess pvAccessCA nt pvData
scanServerRPC_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

