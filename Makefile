CC      = gcc
CFLAGS  = -m64 -I/usr/local/include -DMT_SAFE -D_POSIX_C_SOURCE=200112L
LFLAGS  += -lm -L/usr/local/lib
LSCUT   = -L/usr/local/lib -lscut
OS      = $(shell uname -s)
ISA     = $(shell uname -p)
DEBUG   = 1
LCRYPTO = -lssl -lcrypto
LIMG    = -lpng -ljpeg -lraw
LNET    =

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

ifeq ($(OS), OpenBSD)
  CC = clang
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

ifeq ($(CC), clang)
  CFLAGS += -std=c99 -pedantic -O3 -fstrict-aliasing
  CFLAGS += -Wextra -Wall -Wstrict-aliasing
endif

# Configure based on OS/Compiler
ifeq ($(OS), SunOS)
  LNET = -lnsl -lsocket
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

DIRS       = obj bin obj/lib
IO_SRC     = pie_io_jpg.c pie_io_png.c pie_io.c pie_io_raw.c
UTIL_SRC   = timing.c hmap.c chan.c chan_poll.c lock.c s_queue.c \
	     s_queue_intra.c fswalk.c llist.c strutil.c evp_hw.c fal.c worker.c
ALG_SRC    = pie_hist.c pie_contr.c pie_expos.c pie_curve.c pie_cspace.c \
             pie_satur.c pie_black.c pie_white.c pie_shado.c pie_highl.c \
             pie_unsharp.c pie_vibra.c pie_colort.c pie_medf3.c
ENC_SRC    = pie_json.c pie_rgba.c
MATH_SRC   = pie_math.c pie_catmull.c pie_blur.c pie_kernel.c pie_median.c
BM_SRC     = pie_bm.c pie_render.c
HTTP_SRC   = pie_session.c pie_util.c
CFG_SRC    = pie_cfg.c
DOML_SRC   = pie_doml_stg.c pie_doml_mob.c
EXIF_SRC   = pie_exif.c
DM_SRC     = pie_host.c pie_mountpoint.c pie_storage.c pie_collection.c \
             pie_collection_member.c pie_exif_data.c pie_mob.c pie_min.c \
             pie_dev_params.c
CORE_SRC   = pie_id.c
EDITD_SRC  = pie_editd_ws.c pie_cmd.c pie_wrkspc_mgr.c \
             pie_editd.c pie_msg.c
MEDIAD_SRC = mediad.c new_media.c
INGEST_SRC = ingestd.c file_proc.c
COLLD_SRC  = pie_colld.c pie_coll_handler.c
EXPORTD_SRC = exportd.c

# Core objects
IO_OBJS     = $(IO_SRC:%.c=obj/%.o)
MATH_OBJS   = $(MATH_SRC:%.c=obj/%.o)
BM_OBJS     = $(BM_SRC:%.c=obj/%.o)
ALG_OBJS    = $(ALG_SRC:%.c=obj/%.o)
EXIF_OBJS   = $(EXIF_SRC:%.c=obj/%.o)

# Objects
ENC_OBJS    = $(ENC_SRC:%.c=obj/%.o)
HTTP_OBJS   = $(HTTP_SRC:%.c=obj/%.o)
CFG_OBJS    = $(CFG_SRC:%.c=obj/%.o)
DM_OBJS     = $(DM_SRC:%.c=obj/%.o)
DOML_OBJS   = $(DOML_SRC:%.c=obj/%.o)
UTIL_OBJS   = $(UTIL_SRC:%.c=obj/%.o)

# Server objs
EDITD_OBJS  = $(EDITD_SRC:%.c=obj/%.o)
MEDIAD_OBJS = $(MEDIAD_SRC:%.c=obj/%.o)
INGEST_OBJS = $(INGEST_SRC:%.c=obj/%.o)
COLLD_OBJS  = $(COLLD_SRC:%.c=obj/%.o)
EXPORTD_OBJS = $(EXPORTD_SRC:%.c=obj/%.o)

# Binaries
TEST_BINS   = $(shell find testp -type f | xargs -I {} basename {} ".c")
T_BINS     = $(TEST_BINS:%=bin/%)

VPATH = io lib alg encoding math bm http editd collectiond mediad cfg dm \
        ingestd exif jsmn doml tools exportd

OBJDIR=obj
LIBDIR=$(OBJDIR)/lib

LIBCORE=$(LIBDIR)/libpcore.a
LIBUTIL=$(LIBDIR)/libputil.a
LIBDM=$(LIBDIR)/libpdm.a
LIBHTTP=$(LIBDIR)/libphttp.a

.PHONY: all
.PHONY: test
.PHONY: clean
.PHONY: lint
.PHONY: pied

########################################################################

all: test pied bin/collver

pied: bin/editd bin/ingestd bin/mediad bin/collectiond bin/exportd

test: $(T_BINS)

dir: $(DIRS)

$(DIRS):
	mkdir $(DIRS)

$(LIBCORE): $(IO_OBJS) $(MATH_OBJS) $(ALG_OBJS) $(BM_OBJS) $(EXIF_OBJS) pie_id.o
	ar -cr $@ $(IO_OBJS) $(MATH_OBJS) $(ALG_OBJS) $(BM_OBJS) $(EXIF_OBJS) pie_id.o

$(LIBUTIL): $(UTIL_OBJS) $(ENC_OBJS) obj/jsmn.o
	ar -cr $@ $(UTIL_OBJS) $(ENC_OBJS) obj/jsmn.o

$(LIBDM): $(DM_OBJS) $(DOML_OBJS)
	ar -cr $@ $(DM_OBJS) $(DOML_OBJS)

$(LIBHTTP): $(HTTP_OBJS)
	ar -cr $@ $(HTTP_OBJS)

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o bin/* obj/lib/*.a

bin/pngrw: testp/pngrw.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/pngcreate: testp/pngcreate.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/imgread: testp/imgread.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/jpgcreate: testp/jpgcreate.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/jpgtopng: testp/jpgtopng.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/linvsgma: testp/linvsgma.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/analin: testp/analin.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/histinfo: testp/histinfo.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/contr: testp/contr.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/texp: testp/texp.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/gauss: testp/gauss.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/unsharp: testp/unsharp.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/tojpg: testp/tojpg.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/catm: testp/catm.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/tapply: testp/tapply.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/tdowns: testp/tdowns.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/lrawtest: testp/lrawtest.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/exif_dump: testp/exif_dump.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) -lexif $(LIBCORE) $(LIBUTIL)

bin/tjson: testp/tjson.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIBCORE) $(LIBUTIL)

bin/test_exif_meta: testp/test_exif_meta.c $(LIBCORE) $(CFG_OBJS) $(LIBUTIL) $(LIBDM)
	$(CC) $(CFLAGS) $< $(CFG_OBJS) -o $@ $(LFLAGS) -lexif -lsqlite3 -lraw $(LIBCORE) $(LIBUTIL) $(LIBDM)

bin/bench_blur: testp/bench_blur.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/test_gamma: testp/test_gamma.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIBCORE)

bin/test_id: testp/test_id.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIBCORE)

bin/testfwlk: testp/testfwlk.c $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LCRYPTO) $(LIBUTIL)

bin/qserver: testp/qserver.c $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LNET) -lpthread $(LIBUTIL)

bin/qclient: testp/qclient.c $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LNET) $(LIBUTIL)

bin/test_export: testp/test_export.c $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LNET) $(LIBUTIL)

bin/exif_tags: testp/exif_tags.c
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) -lexif

# Servers
bin/editd: $(EDITD_OBJS) $(CFG_OBJS) $(LIBCORE) $(LIBUTIL) $(LIBDM) $(LIBHTTP)
	$(CC) $(CFLAGS) $(EDITD_OBJS) $(CFG_OBJS) -o $@ $(LFLAGS) -lwebsockets $(LCRYPTO) $(LIMG) -lsqlite3 $(LNET) $(LIBCORE) $(LIBUTIL) $(LIBDM) $(LIBHTTP)

bin/ingestd: $(INGEST_OBJS) $(CFG_OBJS) $(LIBUTIL) $(LIBDM)
	$(CC) $(CFLAGS) $(INGEST_OBJS) $(CFG_OBJS) -o $@ $(LFLAGS) $(LNET) -lsqlite3 $(LCRYPTO) $(LIBUTIL) $(LIBDM)

bin/mediad: $(MEDIAD_OBJS) $(CFG_OBJS) $(LIBCORE) $(LIBUTIL) $(LIBDM)
	$(CC) $(CFLAGS) $(MEDIAD_OBJS) $(CFG_OBJS) -o $@ $(LFLAGS) $(LNET) $(LCRYPTO) -lsqlite3 -lexif $(LIMG) $(LIBCORE) $(LIBDM) $(LIBUTIL)

bin/collectiond: $(COLLD_OBJS) $(CFG_OBJS) $(LIBUTIL) $(LIBDM) $(LIBHTTP)
	$(CC) $(CFLAGS) $(COLLD_OBJS) $(CFG_OBJS) -o $@ $(LFLAGS) $(LNET) -lwebsockets $(LCRYPTO) -lsqlite3 $(LIBUTIL) $(LIBDM) $(LIBHTTP)

bin/exportd: $(EXPORTD_OBJS) $(CFG_OBJS) $(LIBCORE) $(LIBDM) $(LIBUTIL)
	$(CC) $(CFLAGS) $(EXPORTD_OBJS) $(CFG_OBJS) -o $@ $(LFLAGS) $(LNET) -lsqlite3 $(LIMG) $(LIBCORE) $(LIBDM) $(LIBUTIL)

# Tools
bin/collver: tools/collver.c $(LIBCORE) $(CFG_OBJS) $(LIBDM) $(LIBUTIL)
	$(CC) $(CFLAGS) $< $(CFG_OBJS) -o $@ $(LFLAGS) -lsqlite3 $(LIMG) $(LCRYPTO) $(LIBCORE) $(LIBDM) $(LIBUTIL)
