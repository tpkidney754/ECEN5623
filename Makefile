INCLUDE_DIRS =
LIB_DIRS =
host: CC = gcc
de1: CC = arm-linux-gnueabihf-gcc

CDEFS=
CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS=

HFILES=
CFILES= exercise1.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	exercise1

clean:
	-rm -f *.o *.d
	-rm -f perfmon exercise1

distclean:
	-rm -f *.o *.d
	-rm -f exercise1

exercise1: exercise1.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

de1: exercise1

upload: de1
	scp exercise1 root@192.168.1.8:/home/root/bin

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
