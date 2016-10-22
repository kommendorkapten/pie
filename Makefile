CC     = gcc
CFLAGS = -m64 -I/usr/local/include -DMT_SAFE
LFLAGS += -lm -lpng -ljpeg
LSCUT  = -L/usr/local/lib -lscut
OS     = $(shell uname -s)
ISA    = $(shell uname -p)
DEBUG  = 1

# Default to c99 on Solaris
ifeq ($(OS), SunOS)
  CC = c99
  CFLAGS += -D_POSIX_C_SOURCE=200112L
endif

# Configure stuff based on compiler
ifeq ($(CC), gcc)
  CFLAGS += -W -Wall -pedantic -std=c99 -O2
endif

# Configure based on OS/Compiler
ifeq ($(OS), SunOS)
  ifeq ($(CC), c99)
    CFLAGS += -v -xO5 -mt
    ifeq ($(ISA), i386)
      CFLAGS += -xarch=sse4_2 
    endif
  else ifeq ($(CC), gcc)
    ifeq ($(ISA), i386)
      CFLAGS += -march=nehalem
    endif
  endif
else ifeq ($(OS), FreeBSD)
  # libpng and libjpeg is found in /usr/local/lib when installed via ports
  LFLAGS := -L/usr/local/lib $(LFLAGS)
  ifeq ($(CC), gcc)
    CFLAGS += -mtune=$(ISA) -mcpu=$(ISA)
  endif
endif

# Configuration based on ISA
ifeq ($(ISA), i386)
CFLAGS += -D_HAS_SIMD -D_HAS_SSE
else ifeq ($(ISA), powerpc64)
CFLAGS += -D_HAS_SIMD -D_HAS_ALTIVEC 
else ifeq ($(ISA), sparc)
else
endif

ifeq ($(DEBUG), 1)
  CFLAGS += -g -DDEBUG=1
else
  CFLAGS += -DNDEBUG
endif

DIRS     = obj bin
IO_SRC   = pie_io_jpg.c pie_io_png.c pie_io.c
LIB_SRC  = timing.c hmap.c chan.o chan_poll.o lock.o
SRV_SRC  = pie_server.c pie_session.c
MSG_SRC  = pie_msg.c
ALG_SRC  = pie_hist.c
SOURCES  = pie_bm.c pie_cspace.c $(IO_SRC) $(LIB_SRC) $(ALG_SRC) $(MSG_SRC)
OBJS     = $(SOURCES:%.c=obj/%.o)
SRV_OBJS = $(SRV_SRC:%.c=obj/%.o)
BINS     = pngrw pngcreate imgread jpgcreate jpgtopng linvsgma analin \
           histinfo server
P_BINS   = $(BINS:%=bin/%)

VPATH = io lib alg wsrv msg

.PHONY: clean
.PHONY: lint

########################################################################

all: $(OBJS) $(P_BINS)

dir: $(DIRS)

$(DIRS):
	mkdir $(DIRS)

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o bin/* $(P_BINS)

lint:
	lint -Xc99 -m64 -errwarn=%all -errchk=%all -Ncheck=%all -Nlevel=1 -u -m -erroff=E_FUNC_RET_ALWAYS_IGNOR,E_SIGN_EXTENSION_PSBL,E_CAST_INT_TO_SMALL_INT $(SOURCES)

bin/pngrw: testp/pngrw.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/pngcreate: testp/pngcreate.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/imgread: testp/imgread.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/jpgcreate: testp/jpgcreate.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/jpgtopng: testp/jpgtopng.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/linvsgma: testp/linvsgma.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/analin: testp/analin.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/histinfo: testp/histinfo.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/server: testp/server.c $(OBJS) $(SRV_OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(SRV_OBJS) -o $@ -L/usr/local/lib -lwebsockets -lmd $(LFLAGS)
