include sources.mk
INCLUDE_DIRS =
LIB_DIRS =
host: CC = gcc
de1: CC = arm-linux-gnueabihf-gcc
de1: OUT = de1Clock
CDEFS =
CFLAGS = -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS =

OBJS = ${SRCS:.c=.o}
OUT = ${OBJS:.o=.out}

all: clean $(OBJS) ${OUT}

clean:
	rm -f *.o *.d *.out

distclean:
	-rm -f *.o *.d

de1: clean all upload

upload:
	scp *.out ubuntu@192.168.1.23:/home/ubuntu/bin

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c $(INCLUDES) -o $@

%.out: %.o
	$(CC) $(CFLAGS) -o $@ $*.o -lpthread
