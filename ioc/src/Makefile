TOP=../..

include $(TOP)/configure/CONFIG


INC += pv/pvArrayRecord.h

DBD += pvArrayRegister.dbd
DBD += codecIoc.dbd

PROD_IOC += codecIoc


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
codecIoc_SRCS += pvArrayRecord.cpp
codecIoc_SRCS += pvArrayRegister.cpp
codecIoc_SRCS += codecIoc_registerRecordDeviceDriver.cpp
codecIoc_SRCS_DEFAULT += codecIocMain.cpp
codecIoc_SRCS_vxWorks += -nil-

# The following adds support from base/src/vxWorks
codecIoc_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

codecIoc_LIBS += codecRecord
codecIoc_LIBS += blosc
codecIoc_LIBS += pvDatabase pvaClient qsrv pvAccessIOC pvAccess pvAccessCA nt pvData
codecIoc_LIBS += $(EPICS_BASE_IOC_LIBS)


PROD_SYS_LIBS_WIN32 += ws2_32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

