TOP=..

include $(TOP)/configure/CONFIG

codecSRC = $(TOP)/codecRecordSrc

INC += pv/codecRecord.h

DBD += codecRegister.dbd


LIBRARY = codecRecord

# shared library ABI version.
SHRLIB_VERSION ?= 4.3.0

LIBSRCS += codecRecord.cpp
LIBSRCS += codecRegister.cpp


include $(TOP)/configure/RULES


