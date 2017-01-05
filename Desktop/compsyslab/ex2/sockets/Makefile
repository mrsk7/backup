###################################################
#
# Makefile
# Simple TCP/IP communication using sockets
#
# Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
#
###################################################

CC = gcc

CFLAGS = -Wall
CFLAGS += -g
# CFLAGS += -O2 -fomit-frame-pointer -finline-functions

LIBS = 

BINS = socket-server socket-client no_imagination

all: $(BINS)

socket-server: socket-server.c socket-common.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

socket-client: socket-client.c socket-common.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
no_imagination: no_imagination.c socket-common.h
	$(CC) $(CFLAGS) -o server-client

clean:
	rm -f *.o *~ $(BINS)
