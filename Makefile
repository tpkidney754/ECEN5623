INCLUDE_DIRS =
LIB_DIRS =
host: CC = gcc
de1: CC = arm-linux-gnueabihf-gcc
de1: OUT = de1Clock
CDEFS=
CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS=

HFILES=
CFILES= exercise1.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	exercise1 posix_clock

clean:
	rm -f *.o *.d
	rm -f exercise1 posix_clock

distclean:
	-rm -f *.o *.d

exercise1: exercise1.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

posix_clock: posix_clock.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

de1: all

upload: de1
	scp exercise1 ubuntu@192.168.1.20:/home/ubuntu/bin
	scp posix_clock ubuntu@192.168.1.20:/home/ubuntu/bin

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
