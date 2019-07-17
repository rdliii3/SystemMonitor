# Custom Makefile for SystemMonitor

SRCS = main.cpp
HEADERS = constants.h  ProcessContainer.h  Process.h  ProcessParser.h  SysInfo.h  util.h
OBJS = main.o

CFLAGS = -std='c++11'
LIBS = -lncurses

sm: ${OBJS}
	${CXX} -o $@ $? ${CFLAGS} ${LIBS}

main.o: ${SRCS} ${HEADERS}
	${CXX} -c main.cpp ${CFLAGS}

new:
	touch *

clean:
	rm -f *.o
