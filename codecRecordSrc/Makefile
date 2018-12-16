TOP=..

include $(TOP)/configure/CONFIG

codecSRC = $(TOP)/codecRecordSrc

EPICS_BASE_PVA_CORE_LIBS = pvDatabase pvaClient pvAccess pvAccessCA nt pvData ca Com

INC += pv/codecRecord.h
INC += pv/pvArrayRecord.h

DBD += codecRegister.dbd
DBD += pvArrayRegister.dbd


LIBRARY = codecRecord

# shared library ABI version.
SHRLIB_VERSION ?= 4.3.0

LIBSRCS += codecRecord.cpp
LIBSRCS += codecRegister.cpp
LIBSRCS += pvArrayRecord.cpp
LIBSRCS += pvArrayRegister.cpp

codec_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

include $(TOP)/configure/RULES


