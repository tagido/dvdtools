CC:=gcc

host-type := $(shell uname)

ifeq ($(host-type),Linux)
PLATFORM := LINUX
BUILD_DATE := $(shell date --utc)

else

PLATFORM := WIN32
BUILD_DATE := $(shell date /t) $(shell time /t)

endif

ifeq ($(PLATFORM),WIN32)

INCLUDES:= ../lib/tmp/liblist-2.4.mingw32/include
LIBS:= -L../lib/tmp/liblist-2.4.mingw32/lib -lws2_32

PLATFORM_DIR=win32

LIBS:= -lavformat -lavutil -ldvdread
LDFLAGS:=  $(LIBS)
else

# -L../lib/tmp/liblist-2.4.mingw32/lib

#LIBS:= -L../lib/tmp/  -llist 


INCLUDES:= .

PLATFORM_DIR=linux

PKGCONF = pkgconf
PKGCONF_MODULES = dvdread libavformat libavutil
CFLAGS = -Wall -g -fsanitize=address
CFLAGS += `$(PKGCONF) --cflags $(PKGCONF_MODULES)`
LDFLAGS = `$(PKGCONF) --libs $(PKGCONF_MODULES)`

LIBS:= dvdread libavformat libavutil
LDFLAGS:= --libs $(LIBS)

endif

PROGRAMS = dump_ifo dump_file
PROGRAMS += dump_vobu print_vobu fix_vobu
PROGRAMS += rewrite_ifo make_vob
PROGRAMS += print_cell dump_cell
PROGRAMS += print_startcodes

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

dump_ifo: dump_ifo.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

dump_file: dump_file.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

fix_vobu: fix_vobu.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

make_vob: make_vob.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

print_vobu: print_vobu.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

dump_vobu: dump_vobu.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

print_cell: print_cell.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

dump_cell: dump_cell.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

rewrite_ifo: rewrite_ifo.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

print_startcodes: print_startcodes.c common.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)


