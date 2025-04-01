# Makefile for Proxy Lab 
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.
CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

server.o: server.c server.h client.h csapp.h
	$(CC) $(CFLAGS) -c server.c

cache.o: cache.c cache.h client.h server.h csapp.h
	$(CC) $(CFLAGS) -c cache.c

client.o: client.c client.h server.h cache.h csapp.h
	$(CC) $(CFLAGS) -c client.c

proxy: proxy.o csapp.o server.o cache.o client.o
	$(CC) $(CFLAGS) proxy.o csapp.o server.o cache.o client.o -o proxy $(LDFLAGS)

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz

