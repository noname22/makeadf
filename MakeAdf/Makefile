# MakeAdf

LIBDIR=../Lib
LIBNAME=libadf.a

CFLAGS=-I$(LIBDIR) -Wall -pedantic -std=gnu99 -ggdb
LDFLAGS=-L$(LIBDIR) -ladf

PREFIX=/usr/local

EXES=makeadf

all: $(EXES)

lib:
	cd $(LIBDIR) && $(MAKE)

makeadf: lib makeadf.o
	$(CC) $(CFLAGS) makeadf.c -o makeadf $(LDFLAGS)

clean:
	-rm makeadf
	-rm makeadf.o

install:
	cp makeadf $(PREFIX)/bin
