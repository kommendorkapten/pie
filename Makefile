CC      = gcc
CFLAGS  = -m64 -I/usr/local/include -DMT_SAFE
LFLAGS  += -lm -lpng -ljpeg
LSCUT   = -L/usr/local/lib -lscut
OS      = $(shell uname -s)
ISA     = $(shell uname -p)
DEBUG   = 1
LCRYPTO = -lmd

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
    CFLAGS += -v -mt -fast # use -fast and compare to only -xO5
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
  LFLAGS := -L/usr/local/lib $(LFLAGS) -lpthread
  ifeq ($(CC), gcc)
    CFLAGS += -mtune=$(ISA) -mcpu=$(ISA)
  endif
else ifeq ($(OS), Darwin)
  CFLAGS  += -march=native -D_USE_OPEN_SSL
  LCRYPTO = -lcrypto
  LFLAGS  += -L/usr/local/lib
endif

# Configuration based on ISA
ifeq ($(ISA), i386)
CFLAGS += -D_HAS_SIMD -D_HAS_SSE
else ifeq ($(ISA), powerpc64)
CFLAGS += -D_HAS_SIMD_NOT_YET -D_HAS_ALTIVEC 
else ifeq ($(ISA), sparc)
else
endif

ifeq ($(DEBUG), 1)
  CFLAGS += -g -DDEBUG=2
else
  CFLAGS += -DNDEBUG
endif

DIRS      = obj bin
IO_SRC    = pie_io_jpg.c pie_io_png.c pie_io.c
LIB_SRC   = timing.c hmap.c chan.c chan_poll.c lock.c
SRV_SRC   = pie_server.c pie_session.c pie_cmd.c
EXE_SRC   = pie_render.c
MSG_SRC   = pie_msg.c
ALG_SRC   = pie_hist.c pie_contr.c pie_expos.c pie_kernel.c pie_curve.c \
            pie_satur.c pie_black.c pie_white.c pie_shado.c pie_highl.c \
            pie_unsharp.c
ENC_SRC   = pie_json.c
MTH_SRC   = pie_math.c pie_catmull.c
BM_SRC    = pie_bm.c pie_dwn_smpl.c
SOURCES   = pie_cspace.c \
	    $(IO_SRC) $(LIB_SRC) $(ALG_SRC) $(MSG_SRC) $(ENC_SRC) \
            $(MTH_SRC) $(BM_SRC)
OBJS      = $(SOURCES:%.c=obj/%.o)
SRV_OBJS  = $(SRV_SRC:%.c=obj/%.o)
EXE_OBJS  = $(EXE_SRC:%.c=obj/%.o)
TEST_BINS = pngrw pngcreate imgread jpgcreate jpgtopng linvsgma analin \
            histinfo contr gauss unsharp tojpg catm tapply tdowns
EXE_BINS  = server
T_BINS    = $(TEST_BINS:%=bin/%)
E_BINS    = $(EXE_BINS:%=bin/%)

VPATH = io lib alg wsrv msg encoding math exe bm

.PHONY: all
.PHONY: exe
.PHONY: test
.PHONY: clean
.PHONY: lint

########################################################################

all: $(OBJS) exe test

exe: $(E_BINS)

test: $(T_BINS) 

dir: $(DIRS)

$(DIRS):
	mkdir $(DIRS)

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o bin/* $(P_BINS)

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

bin/contr: testp/contr.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/gauss: testp/gauss.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/unsharp: testp/unsharp.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/tojpg: testp/tojpg.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/catm: testp/catm.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

bin/tapply: testp/tapply.c $(OBJS) $(EXE_OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(EXE_OBJS) -o $@ $(LFLAGS)

bin/tdowns: testp/tdowns.c $(OBJS) $(EXE_OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(EXE_OBJS) -o $@ $(LFLAGS)

bin/server: exe/server.c $(OBJS) $(SRV_OBJS) $(EXE_OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(SRV_OBJS) $(EXE_OBJS) -o $@ -L/usr/local/lib -lwebsockets $(LCRYPTO) $(LFLAGS)
