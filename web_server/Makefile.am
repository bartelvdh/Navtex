AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = navtexwebserver
navtexwebserver_SOURCES = ../receiver/message_store.c  https_fileserver.c
AM_CFLAGS= -I/usr/local/lib  -I/usr/local/include
navtexwebserver_LDADD =  -L/usr/local/lib -lcjson -ldl -lm -lsqlite3  -lmicrohttpd 
navtexwebserver_LDFLAGS =  -pthread 

AM_CPPFLAGS = -DDATA_FILE=\"/opt/Navtex/Navtex.db\"


message_store.o: message_store.h

message_store.h:
        ln -s ../receiver/message_store.h message_store.h

