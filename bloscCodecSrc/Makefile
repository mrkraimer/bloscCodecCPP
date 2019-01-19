TOP=..

include $(TOP)/configure/CONFIG

LIBRARY = blosc

LIBBLOSC = $(TOP)/bloscCodecSrc

USR_CFLAGS += -DHAVE_ZSTD
USR_CFLAGS += -DHAVE_SNAPPY
USR_CFLAGS += -DHAVE_LZ4
#USR_CFLAGS += -DHAVE_ZLIB

SRC_DIRS += $(LIBBLOSC)/c-blosc/internal-complibs/zstd-1.3.7
SRC_DIRS += $(LIBBLOSC)/c-blosc/internal-complibs/zstd-1.3.7/common

LIB_SRCS += entropy_common.c
LIB_SRCS += error_private.c
LIB_SRCS += fse_decompress.c
LIB_SRCS += pool.c
LIB_SRCS += threading.c
LIB_SRCS += xxhash.c
LIB_SRCS += zstd_common.c

SRC_DIRS += $(LIBBLOSC)/c-blosc/internal-complibs/zstd-1.3.7/compress
LIB_SRCS += fse_compress.c
LIB_SRCS += zstd_compress.c
LIB_SRCS += huf_compress.c
LIB_SRCS += zstdmt_compress.c
LIB_SRCS += zstd_ldm.c

LIB_SRCS += zstd_opt.c
LIB_SRCS += zstd_fast.c
LIB_SRCS += zstd_lazy.c
LIB_SRCS += hist.c
LIB_SRCS += zstd_double_fast.c
SRC_DIRS += $(LIBBLOSC)/c-blosc/internal-complibs/zstd-1.3.7/decompress
LIB_SRCS += huf_decompress.c
LIB_SRCS += zstd_decompress.c

SRC_DIRS += $(LIBBLOSC)/c-blosc/internal-complibs/snappy-1.1.1
LIB_SRCS += snappy.cc
LIB_SRCS += snappy-c.cc
LIB_SRCS += snappy-sinksource.cc
LIB_SRCS += snappy-stubs-internal.cc

SRC_DIRS += $(LIBBLOSC)/c-blosc/internal-complibs/lz4-1.8.3
LIB_SRCS += lz4.c
LIB_SRCS += lz4hc.c
SRC_DIRS += $(LIBBLOSC)/c-blosc/internal-complibs/zlib-1.2.8
    LIB_SRCS += adler32.c
    LIB_SRCS += compress.c
    LIB_SRCS += crc32.c
    LIB_SRCS += deflate.c
    LIB_SRCS += gzclose.c
    LIB_SRCS += gzlib.c
    LIB_SRCS += gzread.c
    LIB_SRCS += gzwrite.c
    LIB_SRCS += infback.c
    LIB_SRCS += inffast.c
    LIB_SRCS += inflate.c
    LIB_SRCS += inftrees.c
    LIB_SRCS += trees.c
    LIB_SRCS += uncompr.c
    LIB_SRCS += zutil.c


SRC_DIRS += $(LIBBLOSC)/c-blosc/blosc


#INC += bitshuffle-avx2.h
#INC += bitshuffle-generic.h
#INC += bitshuffle-sse2.h
#INC += blosc-common.h
#INC += blosc-comp-features.h
INC += blosc-export.h
INC += blosc.h
#INC += blosclz.h
#INC += fastcopy.h
#INC += shuffle-avx2.h
#INC += shuffle-generic.h
#INC += shuffle.h
#INC += shuffle-sse2.h

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

SRC_DIRS += $(LIBBLOSC)/codecSrc
INC += pv/bloscCodec.h

LIBSRCS += bloscCodec.cpp


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

