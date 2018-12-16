TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library
#

#=============================
# build an ioc application


DBD += codec.dbd

PROD_IOC += codec


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
codec_SRCS += codec_registerRecordDeviceDriver.cpp
codec_SRCS_DEFAULT += codecMain.cpp
codec_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
codec_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

codec_LIBS += codecRecord
codec_LIBS += blosc
codec_LIBS += pvDatabase pvaClient qsrv pvAccessIOC pvAccess pvAccessCA nt pvData
codec_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

