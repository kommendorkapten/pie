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
SOURCES = pie_bm.c pie_cspace.c pie_io_png.c pie_io_jpg.c
OBJS    = $(SOURCES:%.c=obj/%.o)
BINS    = pngrw pngcreate pngread jpgcreate

.PHONY: clean
.PHONY: lint

########################################################################

all: $(OBJS) $(BINS)

dir: $(DIRS)

$(DIRS):
	mkdir $(DIRS)

obj/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/*.o bin/* $(BINS)

lint:
	lint -Xc99 -m64 -errwarn=%all -errchk=%all -Ncheck=%all -Nlevel=1 -u -m -erroff=E_FUNC_RET_ALWAYS_IGNOR,E_SIGN_EXTENSION_PSBL,E_CAST_INT_TO_SMALL_INT $(SOURCES)

pngrw: pngrw.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

pngcreate: pngcreate.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

pngread: pngread.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)

jpgcreate: jpgcreate.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LFLAGS)
