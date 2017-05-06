CC      = gcc
CFLAGS  = -m64 -I/usr/local/include -DMT_SAFE -D_POSIX_C_SOURCE=200112L
LFLAGS  += -lm
LSCUT   = -L/usr/local/lib -lscut
OS      = $(shell uname -s)
ISA     = $(shell uname -p)
DEBUG   = 1
LCRYPTO = -lssl -lcrypto

# -ftree-loop-linear MAY introduce bugs.
PPC_FAST = -falign-functions=16 -falign-loops=16 -falign-jumps=16 \
           -malign-natural -ffast-math -fstrict-aliasing \
           -funroll-loops -ftree-loop-linear \
           -mcpu=970 -mtune=970 -mpowerpc64 -mpowerpc-gpopt -fgcse-sm \
           -mfused-madd -maltivec

# Default to c99 on Solaris
ifeq ($(OS), SunOS)
  CC = c99
endif

ifeq ($(OS), FreeBSD)
  # Something weird with FreeBSD and math.h, M_PI is not visible unless
  # _XOPEN_SOURCE is defined. (_POSIX_C_SOURCE is not needed when
  # _XOPEN_SOURCE but as long as they do not conflict it is not a problem
  # to define both of them).
  CFLAGS += -D_XOPEN_SOURCE=600
endif

# Configure stuff based on compiler
ifeq ($(CC), gcc)
  CFLAGS += -std=c99 -pedantic -O3 -fstrict-aliasing
  CFLAGS += -Wextra -Wall -Wstrict-aliasing
endif

# Configure based on OS/Compiler
ifeq ($(OS), SunOS)
  ifeq ($(CC), cc)
    CFLAGS += -std=c99 -pedantic -v -errwarn -mt -fast -xalias_level=std
  else ifeq ($(CC), c99)
    CFLAGS += -v -errwarn -mt -xalias_level=std -fast 
  else ifeq ($(CC), gcc)
    CFLAGS += -Wconversion -Wno-sign-conversion
  endif
else ifeq ($(OS), FreeBSD)
  # libpng and libjpeg is found in /usr/local/lib when installed via ports
  LFLAGS := -L/usr/local/lib $(LFLAGS) -lpthread
  ifeq ($(CC), gcc)
    CFLAGS += -ffast-math
  endif
else ifeq ($(OS), Darwin)
  ifeq ($(ISA), powerpc)
    CFLAGS  += -fast
  else
    CFLAGS  += -Wconversion -Wno-sign-conversion
    LFLAGS  += -L/usr/local/lib
  endif
endif

include include.$(OS).$(ISA).mk
CFLAGS += $(CFLAGS_$(CC))

ifeq ($(DEBUG), 1)
  CFLAGS += -g -DDEBUG=1
else
  CFLAGS += -DNDEBUG
endif

DIRS       = obj bin
IO_SRC     = pie_io_jpg.c pie_io_png.c pie_io.c
LIB_SRC    = timing.c hmap.c chan.c chan_poll.c lock.c s_queue.c \
	     s_queue_intra.c fswalk.c llist.c strutil.c evp_hw.c fal.c
ALG_SRC    = pie_hist.c pie_contr.c pie_expos.c pie_kernel.c pie_curve.c \
             pie_satur.c pie_black.c pie_white.c pie_shado.c pie_highl.c \
             pie_unsharp.c pie_vibra.c pie_colort.c
ENC_SRC    = pie_json.c pie_rgba.c
MTH_SRC    = pie_math.c pie_catmull.c pie_blur.c
BM_SRC     = pie_bm.c pie_dwn_smpl.c
HTTP_SRC   = pie_session.c pie_util.c
EDITD_SRC  = pie_editd_ws.c pie_cmd.c pie_render.c pie_wrkspc_mgr.c \
             pie_editd.c pie_msg.c
MEDIAD_SRC = mediad.c new_media.c
INGEST_SRC = ingestd.c file_proc.c
COLLD_SRC  = pie_colld.c pie_coll.c
CFG_SRC    = pie_cfg.c
DM_SRC     = pie_host.c pie_mountpoint.c pie_storage.c
SOURCES    = pie_cspace.c pie_id.c\
	     $(IO_SRC) $(ALG_SRC) $(ENC_SRC) $(MTH_SRC) $(BM_SRC)
# Objects
OBJS        = $(SOURCES:%.c=obj/%.o)
HTTP_OBJS   = $(HTTP_SRC:%.c=obj/%.o)
EDITD_OBJS  = $(EDITD_SRC:%.c=obj/%.o)
MEDIAD_OBJS = $(MEDIAD_SRC:%.c=obj/%.o)
INGEST_OBJS = $(INGEST_SRC:%.c=obj/%.o)
COLLD_OBJS  = $(COLLD_SRC:%.c=obj/%.o)
CFG_OBJS    = $(CFG_SRC:%.c=obj/%.o)
DM_OBJS     = $(DM_SRC:%.c=obj/%.o)
TEST_BINS   = pngrw pngcreate imgread jpgcreate jpgtopng linvsgma analin \
              histinfo contr gauss unsharp tojpg catm tapply tdowns test_id \
              testfwlk qserver qclient
T_BINS     = $(TEST_BINS:%=bin/%)

VPATH = io lib alg encoding math bm http editd collectiond mediad cfg dm ingestd

.PHONY: all
.PHONY: test
.PHONY: clean
.PHONY: lint

########################################################################

all: $(OBJS) test bin/editd bin/collectiond bin/mediad bin/ingestd

test: $(T_BINS) 

dir: $(DIRS)

$(DIRS):
	mkdir $(DIRS)

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o bin/* $(P_BINS)

bin/pngrw: testp/pngrw.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS) -lpng -ljpeg

bin/pngcreate: testp/pngcreate.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS) -lpng -ljpeg

bin/imgread: testp/imgread.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS) -lpng -ljpeg

bin/jpgcreate: testp/jpgcreate.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS) -lpng -ljpeg

bin/jpgtopng: testp/jpgtopng.c $(OBJS) obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lpng -ljpeg

bin/linvsgma: testp/linvsgma.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS) -lpng -ljpeg

bin/analin: testp/analin.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS) -lpng -ljpeg

bin/histinfo: testp/histinfo.c $(OBJS) obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lpng -ljpeg

bin/contr: testp/contr.c obj/pie_io.o obj/pie_io_jpg.o obj/pie_io_png.o obj/pie_contr.o obj/pie_bm.o obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lpng -ljpeg

bin/gauss: testp/gauss.c $(OBJS) obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lpng -ljpeg

bin/unsharp: testp/unsharp.c $(OBJS) obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lpng -ljpeg

bin/tojpg: testp/tojpg.c $(OBJS) obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lpng -ljpeg

bin/catm: testp/catm.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS) -lpng -ljpeg

bin/tapply: testp/tapply.c $(OBJS) obj/pie_render.o obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lpng -ljpeg

bin/tdowns: testp/tdowns.c $(OBJS) obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lpng -ljpeg

bin/bench_blur: testp/bench_blur.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS) -lpng -ljpeg

bin/test_id: testp/test_id.c obj/pie_id.o
	$(CC) $(CFLAGS) $< obj/pie_id.o -o $@ $(LFLAGS)

bin/testfwlk: testp/testfwlk.c obj/llist.o obj/fswalk.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LCRYPTO)

bin/qserver: testp/qserver.c obj/s_queue.o obj/s_queue_intra.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lnsl -lsocket

bin/qclient: testp/qclient.c obj/s_queue.o obj/s_queue_intra.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lnsl -lsocket

bin/editd: $(EDITD_OBJS) $(HTTP_OBJS) $(OBJS) obj/hmap.o obj/timing.o obj/chan.o obj/chan_poll.o obj/lock.o
	$(CC) $(CFLAGS) $^ -o $@ -L/usr/local/lib -lwebsockets $(LCRYPTO) $(LFLAGS) -lpng -ljpeg

bin/ingestd: $(INGEST_OBJS) obj/s_queue.o obj/s_queue_intra.o obj/fswalk.o obj/llist.o $(DM_OBJS) $(CFG_OBJS) obj/strutil.o obj/hmap.o obj/chan.o obj/chan_poll.o obj/lock.o obj/evp_hw.o obj/fal.o obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lnsl -lsocket -lsqlite3 $(LCRYPTO)

bin/mediad: $(MEDIAD_OBJS) obj/s_queue.o obj/s_queue_intra.o obj/chan.o obj/chan_poll.o obj/lock.o $(CFG_OBJS) obj/strutil.o $(DM_OBJS) obj/hmap.o obj/evp_hw.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lnsl -lsocket $(LCRYPTO) -lsqlite3

bin/collectiond: $(COLLD_OBJS) $(HTTP_OBJS) obj/hmap.o
	$(CC) $(CFLAGS) $(COLLD_OBJS) $(HTTP_OBJS) obj/hmap.o -o $@ -L/usr/local/lib -lwebsockets $(LCRYPTO) $(LFLAGS)
