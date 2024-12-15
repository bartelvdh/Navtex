CC=gcc
CFLAGS= -O3   -Wall  

all: https_fileserver

clean:
	rm -f *.o  https_fileserver
message_store.o: message_store.c message_store.h
	$(CC) $(CFLAGS) -c message_store.c
https_fileserver.o: https_fileserver.c message_store.h
	$(CC) $(CFLAGS) -c https_fileserver.c
https_fileserver: https_fileserver.o message_store.o message_store.o 
	$(CC) $(CFLAGS) -pthread  https_fileserver.o message_store.o -lmicrohttpd  -lsqlite3 -lcjson -o https_fileserver
