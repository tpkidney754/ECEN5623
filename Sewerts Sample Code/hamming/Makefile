INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= hamming.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	hamming

clean:
	-rm -f *.o *.d
	-rm -f hamming

distclean:
	-rm -f *.o *.d
	-rm -f hamming

pthread: pthread.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
