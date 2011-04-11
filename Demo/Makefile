# unadf

LIBDIR=../Lib
LIBNAME=libadf.a

DEPEND=makedepend

CFLAGS=-I$(LIBDIR) -Wall -Wno-uninitialized -pedantic -std=gnu99 -ggdb
LDFLAGS=-L$(LIBDIR) -ladf

EXES= unadf


all: $(EXES)

run: $(EXES)
	unadf -h
	unadf -lrcs -v 1 ../../hd.adf

lib:
	cd $(LIBDIR) && $(MAKE)

unadf: lib unadf.o
	$(CC) $(CFLAGS) -o $@ unadf.o $(LDFLAGS)

clean:
	(rm *.o $(EXES) core newdev *.exe *~) || true
	echo >.depend

dep:
	$(DEPEND) -f.depend -v -- $(CFLAGS) --  *.[ch]


include .depend
