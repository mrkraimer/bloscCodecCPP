TOP=..

include $(TOP)/configure/CONFIG

LIBRARY = blosc

LIBBLOSC = $(TOP)/bloscSrc

SRC_DIRS = $(LIBBLOSC)/src

INC += bitshuffle-avx2.h
INC += bitshuffle-generic.h
INC += bitshuffle-sse2.h
INC += blosc-common.h
INC += blosc-comp-features.h
INC += blosc-export.h
INC += blosc.h
INC += blosclz.h
INC += fastcopy.h
INC += shuffle-avx2.h
INC += shuffle-generic.h
INC += shuffle.h
INC += shuffle-sse2.h

SRC_DIRS = $(LIBBLOSC)/src

LIBSRCS += blosc.c
LIBSRCS += blosclz.c
LIBSRCS += shuffle-generic.c
LIBSRCS += bitshuffle-generic.c
LIBSRCS += shuffle.c
LIBSRCS += shuffle-sse2.c
LIBSRCS += bitshuffle-sse2.c
#LIBSRCS += shuffle-avx2.c
#LIBSRCS += bitshuffle-avx2.c

LIBSRCS += fastcopy.c





#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

