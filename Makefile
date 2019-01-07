# Makefile at top of application tree

TOP = .
include $(TOP)/configure/CONFIG

DIRS += configure

DIRS += codecBloscSrc
codecBloscSrc_DEPEND_DIRS = configure

DIRS += codecRecordSrc
codecRecordSrc_DEPEND_DIRS = configure

DIRS += ioc
ioc_DEPEND_DIRS = configure

DIRS += test
test_DEPEND_DIRS = configure

DIRS += iocBoot
iocBoot_DEPEND_DIRS = configure

include $(TOP)/configure/RULES_TOP


