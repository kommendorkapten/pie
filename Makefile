CC     = gcc
CFLAGS = -m64 -I/usr/local/include
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
    CFLAGS += -v -xO5
    ifeq ($(ISA), i386)
      CFLAGS += -xarch=sse4_2
    endif
  else ifeq ($(CC), gcc)
    ifeq ($(ISA), i386)
      CFLAGS += -march=nehalem
    endif
  endif
else ifeq ($(OS), FreeBSD)
  ifeq ($(CC), gcc)
    CFLAGS += -mtune=$(ISA) -mcpu=$(ISA)
  endif
endif

ifeq ($(DEBUG), 1)
  CFLAGS += -g
else
  CFLAGS += -DNDEBUG
endif

DIRS    = obj bin
IO_SRC  = pie_io_jpg.c pie_io_png.c pie_io.c
SOURCES = pie_bm.c pie_cspace.c $(IO_SRC)
OBJS    = $(SOURCES:%.c=obj/%.o)
BINS    = pngrw pngcreate pngread jpgcreate jpgtopng
P_BINS  = $(BINS:%=bin/%)

VPATH = io

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

bin/pngrw: pngrw.c obj/pie_io_png.o obj/pie_bm.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

bin/pngcreate: pngcreate.c 
	$(CC) $(CFLAGS) $^ $(OBJS) -o $@ $(LFLAGS)

bin/pngread: pngread.c obj/pie_io_png.o obj/pie_bm.o obj/pie_io.o obj/pie_io_jpg.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

bin/jpgcreate: jpgcreate.c obj/pie_io_jpg.o obj/pie_bm.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

bin/jpgtopng: jpgtopng.c obj/pie_io_jpg.o obj/pie_io_png.o obj/pie_bm.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)
