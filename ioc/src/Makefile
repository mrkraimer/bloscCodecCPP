TOP=../..

include $(TOP)/configure/CONFIG


INC += pv/pvArrayRecord.h

DBD += pvArrayRegister.dbd
DBD += bloscCodecIoc.dbd

PROD_IOC += bloscCodecIoc


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
bloscCodecIoc_SRCS += pvArrayRecord.cpp
bloscCodecIoc_SRCS += pvArrayRegister.cpp
bloscCodecIoc_SRCS += bloscCodecIoc_registerRecordDeviceDriver.cpp
bloscCodecIoc_SRCS_DEFAULT += bloscCodecIocMain.cpp
bloscCodecIoc_SRCS_vxWorks += -nil-

# The following adds support from base/src/vxWorks
bloscCodecIoc_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

bloscCodecIoc_LIBS += bloscCodecRecord
bloscCodecIoc_LIBS += blosc
bloscCodecIoc_LIBS += pvDatabase pvaClient qsrv pvAccessIOC pvAccess pvAccessCA nt pvData
bloscCodecIoc_LIBS += $(EPICS_BASE_IOC_LIBS)


PROD_SYS_LIBS_WIN32 += ws2_32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

