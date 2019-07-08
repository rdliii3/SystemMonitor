# Custom Makefile for SystemMonitor

SRCS = main.cpp

CFLAGS = -std='c++11'
LIBS = -lncurses

sm: main.cpp
	${CXX} -o $@ $? ${CFLAGS} ${LIBS}
