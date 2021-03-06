# ADFLib Makefile for Unix platforms
# tested on Solaris 2.6 and Linux 2.0.36 (RedHat 5.2) 


# real devices routines location
NATIV_DIR = ./Generic

VER=0.7.11
LIBS=libadf.a
SHAREDLIB=libadf.so

AR=ar rc
RANLIB=ranlib
TAR=tar

DEFINES= 

CFLAGS=$(DEFINES) -I${NATIV_DIR} -I.. -I. -Wall -ggdb -std=gnu99

CC=gcc 
DEPEND=makedepend

LDFLAGS=-L. -ladf
LDSHARED=$(CC)

OBJS=	 adf_hd.o adf_disk.o adf_raw.o adf_bitm.o adf_dump.o\
        adf_util.o adf_env.o adf_nativ.o adf_dir.o adf_file.o adf_cache.o \
        adf_link.o adf_salv.o

libadf.a: $(OBJS)
	$(AR) $@ $(OBJS)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

$(SHAREDLIB).$(VER): $(OBJS)
	$(LDSHARED) -o $@ $(OBJS)
	rm -f $(SHAREDLIB) $(SHAREDLIB).1
	ln -s $@ $(SHAREDLIB)
	ln -s $@ $(SHAREDLIB).1


# to define LITT_ENDIAN on little endian machines (intel)
# checks for sizeof(long)=4, sizeof(short)=2, sizeof(int)=4
defendian.h: myconf 
	(./myconf || true)

adf_nativ.o: ${NATIV_DIR}/adf_nativ.c ${NATIV_DIR}/adf_nativ.h
	$(CC) ${CFLAGS} -c ${NATIV_DIR}/adf_nativ.c

clean:
	(rm $(OBJS) $(EXE) defendian.h $(LIBS) $(SHAREDLIB) *~ Win32/*~ Generic/*~) || true
	echo >.depend

src:
	@rm adflib.zip
	@zip -9r adflib.zip *.h *.c Makefile myconf Docs Linux Win32 Refs runtest

dep: defendian.h
	$(DEPEND) -f.depend -v -- $(CFLAGS) --  *.[ch]

include .depend
