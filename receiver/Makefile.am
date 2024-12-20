bin_PROGRAMS = navtexreceiver
navtexreceiver_SOURCES = capt_sched.c  message_store.c  decoder.C  fir1cpp.C  fir2cpp.C  fir3cpp.C  nav_b_sm.C  nav_sched.C
AM_CFLAGS= -I/usr/local/lib  -I/usr/local/include
navtexreceiver_LDADD =  -L/usr/local/lib -lsdrplay_api  -lcjson -ldl -lm -lsqlite3
navtexreceiver_LDFLAGS =  -pthread 
pkgdata_DATA = Navtex.db

# Distribute the script with the source package
EXTRA_DIST = generate_db.sh

# Rule to build the empty database
all-local: Navtex.db

Navtex.db:
	@echo "Generating empty SQLite database..."
	./generate_db.sh

# Ensure the database is generated before installation
install-data-hook: all-local

