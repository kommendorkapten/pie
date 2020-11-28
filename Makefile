include include.mk

ifeq ($(DEBUG), 1)
  CFLAGS += -g -DDEBUG=1
else
  CFLAGS += -DNDEBUG
endif

# Binaries
#TEST_BINS   = $(shell find testp -type f "*.c" | xargs -I {} basename {} ".c$")
#T_BINS     = $(TEST_BINS:%=bin/%)

#VPATH = io lib alg encoding math bm http editd collectiond mediad cfg dm \
#        ingestd exif jsmn doml tools exportd

.PHONY: all
.PHONY: test
.PHONY: clean
.PHONY: libs
.PHONY: pied

########################################################################

all: test pied bin/collver

libs: vendor/libvendor.a math/libpmath.a mh/libmh.a alg/libalg.a \
	dm/libdm.a bm/libbm.a exif/libpexif.a encoding/libpencoding.a \
	prunt/libprunt.a http/libphttp.a

vendor/libvendor.a:
	cd vendor && $(MAKE)
math/libpmath.a:
	cd math && $(MAKE)
mh/libmh.a:
	cd mh && $(MAKE)
bm/libbm.a:
	cd bm && $(MAKE)
alg/libalg.a:
	cd alg && $(MAKE)
dm/libdm.a:
	cd dm && $(MAKE)
exif/libpexif.a:
	cd exif && $(MAKE)
encoding/libpencoding.a:
	cd encoding && $(MAKE)
prunt/libprunt.a:
	cd prunt && $(MAKE)
http/libphttp.a:
	cd http && $(MAKE)

pied: bin/editd bin/ingestd bin/mediad bin/collectiond bin/exportd

test: $(T_BINS)

dir: $(DIRS)

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	cd vendor && $(MAKE) clean
	cd math && $(MAKE) clean
	cd mh && $(MAKE) clean
	cd bm && $(MAKE) clean
	cd alg && $(MAKE) clean
	cd dm && $(MAKE) clean
	cd exif && $(MAKE) clean
	cd encoding && $(MAKE) clean
	cd prunt && $(MAKE) clean
	cd http && $(MAKE) clean

bin/pngrw: testp/pngrw.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/gradient: testp/gradient.c $(LIBCORE)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE)

bin/imgread: testp/imgread.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/gauss: testp/gauss.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/unsharp: testp/unsharp.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

bin/tojpg: testp/tojpg.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

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

bin/bench_blur: testp/bench_blur.c $(LIBCORE) $(LIBUTIL)
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) $(LIBCORE) $(LIBUTIL)

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

bin/srgb_test: testp/srgb_test.c obj/pie_cspace.o obj/timing.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

# Servers
bin/editd: $(EDITD_OBJS) $(IO_OBJS) $(HTTP_OBJS) $(ALG_OBJS) $(MATH_OBJS) $(DOML_OBJS) $(CFG_OBJS) $(ENC_OBJS) $(BM_OBJS) $(DM_OBJS) obj/llist.o obj/hmap.o obj/chan.o obj/chan_poll.o obj/lock.o obj/strutil.o obj/timing.o obj/s_queue.o obj/s_queue_intra.o obj/pie_id.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lwebsockets $(LCRYPTO) $(LIMG) -lsqlite3 $(LNET)

bin/ingestd: $(INGEST_OBJS) $(CFG_OBJS) $(DM_OBJS) obj/llist.o obj/hmap.o obj/s_queue.o obj/s_queue_intra.o obj/timing.o obj/strutil.o obj/fal.o obj/evp_hw.o obj/fswalk.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LNET) -lsqlite3 $(LCRYPTO)

bin/mediad: $(MEDIAD_OBJS) $(CFG_OBJS) $(DM_OBJS) $(IO_OBJS) $(MATH_OBJS) $(ALG_OBJS) obj/llist.o obj/hmap.o obj/strutil.o obj/timing.o obj/chan.o obj/chan_poll.o obj/lock.o obj/pie_bm.o obj/evp_hw.o obj/s_queue.o obj/s_queue_intra.o obj/pie_exif.o obj/pie_id.o obj/pie_cspace.o obj/btree.o obj/pie_json.o obj/pie_render.o obj/jsmn.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LNET) $(LCRYPTO) -lsqlite3 -lexif $(LIMG)

bin/collectiond: $(COLLD_OBJS) $(CFG_OBJS) $(DM_OBJS) $(DOML_OBJS) $(ENC_OBJS) $(HTTP_OBJS) obj/llist.o obj/hmap.o obj/strutil.o obj/s_queue.o obj/s_queue_intra.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LNET) -lwebsockets $(LCRYPTO) -lsqlite3

bin/exportd: $(EXPORTD_OBJS) $(CFG_OBJS) $(DM_OBJS) $(ALG_OBJS) $(MATH_OBJS) $(BM_OBJS) $(IO_OBJS) $(ENC_OBJS) $(DOML_OBJS) obj/llist.o obj/hmap.o obj/timing.o obj/strutil.o obj/s_queue.o obj/s_queue_intra.o obj/fal.o obj/worker.o obj/chan.o obj/chan_poll.o obj/lock.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LNET) -lsqlite3 $(LIMG)

# Tools
bin/collver: tools/collver.c $(CFG_OBJS) $(DM_OBJS) $(IO_OBJS) $(MATH_OBJS) obj/llist.o obj/hmap.o obj/pie_bm.o obj/pie_cspace.o obj/strutil.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -lsqlite3 $(LIMG) $(LCRYPTO)
