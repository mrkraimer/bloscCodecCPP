# Makefile at top of application tree

TOP = .
include $(TOP)/configure/CONFIG

DIRS += configure

DIRS += bloscCodecSrc
bloscCodecSrc_DEPEND_DIRS = configure

DIRS += bloscCodecRecordSrc
bloscCodecRecordSrc_DEPEND_DIRS = configure

DIRS += ioc
ioc_DEPEND_DIRS = configure

DIRS += iocBoot
iocBoot_DEPEND_DIRS = configure

DIRS += exampleClient
exampleClient_DEPEND_DIRS = configure

include $(TOP)/configure/RULES_TOP


