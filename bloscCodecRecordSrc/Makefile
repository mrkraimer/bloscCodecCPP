TOP=..

include $(TOP)/configure/CONFIG

codecSRC = $(TOP)/bloscCodecRecordSrc

INC += pv/bloscCodecRecord.h

DBD += bloscCodecRecordRegister.dbd


LIBRARY = bloscCodecRecord

# shared library ABI version.
SHRLIB_VERSION ?= 4.3.0

LIBSRCS += bloscCodecRecord.cpp
LIBSRCS += bloscCodecRecordRegister.cpp


include $(TOP)/configure/RULES


