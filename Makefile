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
else ifeq ($(OS), Darwin)
  CFLAGS += -mtune=native -mcpu=native
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

DIRS      = obj bin
IO_SRC    = pie_io_jpg.c pie_io_png.c pie_io.c
LIB_SRC   = timing.c hmap.c chan.c chan_poll.c lock.c
SRV_SRC   = pie_server.c pie_session.c pie_cmd.c
MSG_SRC   = pie_msg.c
ALG_SRC   = pie_hist.c pie_contr.c pie_expos.c
ENC_SRC   = pie_json.c
SOURCES   = pie_render.c pie_bm.c pie_cspace.c \
	    $(IO_SRC) $(LIB_SRC) $(ALG_SRC) $(MSG_SRC) $(ENC_SRC)
OBJS      = $(SOURCES:%.c=obj/%.o)
SRV_OBJS  = $(SRV_SRC:%.c=obj/%.o)
TEST_BINS = pngrw pngcreate imgread jpgcreate jpgtopng linvsgma analin \
            histinfo contr
EXE_BINS  = server
T_BINS    = $(TEST_BINS:%=bin/%)
E_BINS    = $(EXE_BINS:%=bin/%)
LINT_SRC  = $(shell find . -name '*.c')

VPATH = io lib alg wsrv msg encoding

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

lint:
	lint -I/usr/local/include -Xc99 -m64 -errwarn=%all -errchk=%all -Ncheck=%all -Nlevel=1 -u -m -erroff=E_FUNC_RET_ALWAYS_IGNOR,E_SIGN_EXTENSION_PSBL,E_CAST_INT_TO_SMALL_INT $(LINT_SRC)

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

bin/server: exe/server.c $(OBJS) $(SRV_OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(SRV_OBJS) -o $@ -L/usr/local/lib -lwebsockets -lmd $(LFLAGS)
