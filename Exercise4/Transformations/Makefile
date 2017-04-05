INCLUDE_DIRS =
LIB_DIRS =
CC=g++

CDEFS=
CFLAGS= -O0 -pg -g $(INCLUDE_DIRS) $(CDEFS)
LIBS=-lpthread -lrt
CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video

HFILES=fps_capture.hpp
CFILES=
CPPFILES= fps_capture.cpp

SRCS= ${HFILES} ${CFILES}
CPPOBJS= ${CPPFILES:.cpp=.o}

all:	fps_capture

clean:
	-rm -f *.o *.d
	-rm -f fps_capture

distclean:
	-rm -f *.o *.d

fps_capture: fps_capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o `pkg-config --libs opencv` $(CPPLIBS) $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CC) $(CFLAGS) -c $<
