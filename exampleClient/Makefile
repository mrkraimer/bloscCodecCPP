TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pvAccessCA nt pvData ca Com
#EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pva2ca nt pvData ca Com

PROD_HOST += clientGetputCodec
clientGetputCodec_SRCS += clientGetputCodec.cpp
clientGetputCodec_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += clientMonitorCodec
clientMonitorCodec_SRCS += clientMonitorCodec.cpp
clientMonitorCodec_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)


PROD_HOST += clientCodec
clientCodec_SRCS += clientCodec.cpp
clientCodec_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)
clientCodec_LIBS += blosc

PROD_HOST += clientPutArray
clientPutArray_SRCS += clientPutArray.cpp
clientPutArray_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)


PROD_SYS_LIBS_WIN32 += ws2_32


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

