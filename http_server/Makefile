CC=gcc
###CFLAGS=-O -Wall
###CFLAGS= -O3 -Wall -Wl,-rpath=/usr/local/lib64
CFLAGS= -O3 -ffunction-sections  -Wall -Wl,-rpath=/usr/local/lib64 -Wl,-gc-sections  -std=gnu99
LDLIBS=-lsdrplay_api -lonion -lcjson 

all: https_fileserver

clean:
	rm -f *.o  https_fileserver
sqlite3.o: sqlite3.c
	$(CC) $(CFLAGS) -c sqlite3.c
message_store.o: message_store.c message_store.h
	$(CC) $(CFLAGS) -c message_store.c
https_fileserver.o: https_fileserver.c message_store.h
	$(CC) $(CFLAGS) -c https_fileserver.c
https_fileserver: https_fileserver.o message_store.o message_store.o sqlite3.o 
	$(CC) $(CFLAGS) https_fileserver.o message_store.o sqlite3.o -lmicrohttpd -pthread -lcjson -ldl  -o https_fileserver
