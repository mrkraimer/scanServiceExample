TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pvAccessCA nt pvData ca Com
#EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pva2ca nt pvData ca Com

PROD_HOST += scanClientRPC
scanClientRPC_SRCS += scanClientRPC.cpp
scanClientRPC_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

