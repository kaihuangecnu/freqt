CC = gcc
CXX = c++
EXECPREFIX =
VERSION = 0.22
CFLAGS = -O3 -Wall
CXXFLAGS = -O3 -Wall
TARGETS = freqt${EXEC_PREFIX}
OBJ = freqt.o

all: ${OBJ}
	${CXX} ${CFLAGS} ${LDFLAGS} -o ${TARGETS} ${OBJ} ${LDFLAGS}
clean:
	rm -f *.o ${TARGETS} core *~ *.tar.gz *.exe

dist:	
	rm -fr freqt-${VERSION}	
	mkdir freqt-${VERSION}
	cp *.cpp Makefile README AUTHORS COPYING index.html freqt.css data freqt-${VERSION}
	tar zcfv freqt-${VERSION}.tar.gz freqt-${VERSION}
	rm -fr freqt-${VERSION}
	
test:	
	./freqt -m 3 < data
