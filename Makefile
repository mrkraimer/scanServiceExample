# Makefile at top of application tree

TOP = .
include $(TOP)/configure/CONFIG

DIRS += configure

DIRS += scanService
scanService_DEPEND_DIRS = configure

DIRS += scanServerRPC
scanServerRPC_DEPEND_DIRS = configure

DIRS += scanServerPutGet
scanServerPutGet_DEPEND_DIRS = configure

DIRS += scanClientRPC
scanClientRPC_DEPEND_DIRS = configure

DIRS += scanClientPutGet
scanClientPutGet_DEPEND_DIRS = configure

DIRS += ioc
ioc_DEPEND_DIRS = src

DIRS += iocBoot
iocBoot_DEPEND_DIRS = src


include $(TOP)/configure/RULES_TOP


