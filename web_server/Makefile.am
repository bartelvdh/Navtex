AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = navtexwebserver
navtexwebserver_SOURCES = ../receiver/message_store.c  https_fileserver.c 
AM_CFLAGS= -I/usr/local/lib  -I/usr/local/include
navtexwebserver_LDADD =  -L/usr/local/lib -lcjson -ldl -lm -lsqlite3  -lmicrohttpd 
navtexwebserver_LDFLAGS =  -pthread 

AM_CPPFLAGS =  -DDATA_FILE=\"/var/lib/navtex/Navtex.db\"  -DWEB_ROOT=\"/usr/share/navtex/www\"

EXTRA_DIST = build

navtexwwwdir = $(datadir)/navtex/www

install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(navtexwwwdir)
	cp -R $(srcdir)/build/. $(DESTDIR)$(navtexwwwdir)/
