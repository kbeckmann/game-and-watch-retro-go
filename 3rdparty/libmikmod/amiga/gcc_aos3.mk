# makefile fragment for m68k-amigaos / gcc

#CRT_FLAGS = -mcrt=clib2
CRT_FLAGS = -noixemul

LDFLAGS+= $(CRT_FLAGS)
LDLIBS += -lm
CFLAGS += $(CRT_FLAGS)
CPPFLAGS+= -DWORDS_BIGENDIAN=1
