include include.mk

ifeq ($(DEBUG), 1)
  CFLAGS += -g -DDEBUG=1
else
  CFLAGS += -DNDEBUG
endif

# Binaries
TEST_BINS   = $(shell find testp -type f -name "*.c" | xargs -I {} basename {} ".c$")
T_BINS     = $(TEST_BINS:%=bin/%)

#VPATH = io lib alg encoding math bm http editd collectiond mediad cfg dm \
#        ingestd exif jsmn doml tools exportd

.PHONY: all
.PHONY: test
.PHONY: clean
.PHONY: libs
.PHONY: pied
.PHONY: vendor/libvendor.a
.PHONY: math/libpmath.a
.PHONY: mh/libmh.a
.PHONY: bm/libbm.a
.PHONY: alg/libalg.a
.PHONY: dm/libdm.a
.PHONY: exif/libpexif.a
.PHONY: encoding/libpencoding.a
.PHONY: prunt/libprunt.a
.PHONY: http/libphttp.a
.PHONY: editobj

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
editdobj:
	cd editd && $(MAKE)

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
	cd editd && $(MAKE) clean
	rm -f bin/*

bin/pngrw: testp/pngrw.c bm/libbm.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a

bin/gradient: testp/gradient.c bm/libbm.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a

bin/imgread: testp/imgread.c bm/libbm.a vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a vendor/libvendor.a

bin/gauss: testp/gauss.c bm/libbm.a vendor/libvendor.a math/libpmath.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a vendor/libvendor.a math/libpmath.a

bin/unsharp: testp/unsharp.c bm/libbm.a vendor/libvendor.a alg/libalg.a math/libpmath.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a vendor/libvendor.a alg/libalg.a math/libpmath.a

bin/tojpg: testp/tojpg.c bm/libbm.a alg/libalg.a math/libpmath.a vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a alg/libalg.a math/libpmath.a vendor/libvendor.a

bin/tapply: testp/tapply.c bm/libbm.a alg/libalg.a vendor/libvendor.a encoding/libpencoding.a math/libpmath.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a alg/libalg.a vendor/libvendor.a encoding/libpencoding.a math/libpmath.a

bin/tdowns: testp/tdowns.c bm/libbm.a alg/libalg.a math/libpmath.a vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a alg/libalg.a math/libpmath.a vendor/libvendor.a

bin/lrawtest: testp/lrawtest.c bm/libbm.a vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a vendor/libvendor.a

bin/exif_dump: testp/exif_dump.c exif/libpexif.a encoding/libpencoding.a vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) -lexif exif/libpexif.a encoding/libpencoding.a vendor/libvendor.a

bin/tjson: testp/tjson.c alg/libalg.a math/libpmath.a encoding/libpencoding.a vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) alg/libalg.a math/libpmath.a encoding/libpencoding.a vendor/libvendor.a

bin/bench_blur: testp/bench_blur.c bm/libbm.a math/libpmath.a vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LIMG) bm/libbm.a math/libpmath.a vendor/libvendor.a

bin/test_id: testp/test_id.c prunt/libprunt.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) prunt/libprunt.a

bin/testfwlk: testp/testfwlk.c vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LCRYPTO) vendor/libvendor.a

bin/qserver: testp/qserver.c vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LNET) -lpthread vendor/libvendor.a

bin/qclient: testp/qclient.c vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LNET) vendor/libvendor.a

bin/test_export: testp/test_export.c encoding/libpencoding.a vendor/libvendor.a
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) $(LNET) encoding/libpencoding.a vendor/libvendor.a

bin/exif_tags: testp/exif_tags.c
	$(CC) $(CFLAGS) $< -o $@ $(LFLAGS) -lexif

bin/srgb_test: testp/srgb_test.c bm/libbm.a vendor/libvendor.a
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) bm/libbm.a vendor/libvendor.a

# Servers
editdsrc=$(wildcard editd/*.c)
editdobj=$(subst .c,.o,$(editdsrc))
bin/editd: editdobj bm/libbm.a dm/libdm.a alg/libalg.a http/libphttp.a prunt/libprunt.a math/libpmath.a mh/libmh.a encoding/libpencoding.a vendor/libvendor.a
	$(CC) $(CFLAGS) -o $@ $(editdobj) $(LFLAGS) -lwebsockets $(LCRYPTO) $(LIMG) -lsqlite3 $(LNET) bm/libbm.a alg/libalg.a http/libphttp.a prunt/libprunt.a math/libpmath.a mh/libmh.a encoding/libpencoding.a dm/libdm.a vendor/libvendor.a

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
