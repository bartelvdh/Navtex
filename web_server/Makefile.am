AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = navtexwebserver
navtexdir = /opt/Navtex/
navtexwebserver_SOURCES = ../receiver/message_store.c  https_fileserver.c 
AM_CFLAGS= -I/usr/local/lib  -I/usr/local/include
navtexwebserver_LDADD =  -L/usr/local/lib -lcjson -ldl -lm -lsqlite3  -lmicrohttpd 
navtexwebserver_LDFLAGS =  -pthread 

AM_CPPFLAGS =  -DDATA_FILE=\"/opt/Navtex/Navtex.db\"  -DWEB_ROOT=\"/opt/Navtex/build\" 

EXTRA_DIST = build

install-data-hook: 
	@cp -r  build $(navtexdir)/build
