CC=gcc
###CFLAGS=-O -Wall
CFLAGS= -O3 -Wall -Wl,-rpath=/usr/local/lib64
LDLIBS=-lsdrplay_api  -lcjson -pthread -ldl

all:  nav_sched 

clean:
	rm -f *.o  dumpiqw makepng nav
nav_sched.o: nav_sched.c fir1.h
	$(CC) $(CFLAGS) -c nav_sched.c
nav_sched: capt_sched.o nav_sched.o fir1.o fir2.o fir3.o quad_mod.o bit_sync.o bit_det.o navtex_bytes_sm.o message_store.o sqlite3.o 
	$(CC) $(CFLAGS) capt_sched.o nav_sched.o fir1.o fir2.o fir3.o quad_mod.o bit_sync.o bit_det.o navtex_bytes_sm.o  message_store.o sqlite3.o -o nav_sched -lm $(LDLIBS)
capt_sched.o: capt_sched.c
	$(CC) $(CFLAGS) -c capt_sched.c
sqlite3.o: sqlite3.c
	$(CC) $(CFLAGS) -c sqlite3.c
message_store.o: message_store.c message_store.h
	$(CC) $(CFLAGS) -c message_store.c
fir1.o: fir1.c
	$(CC) $(CFLAGS) -c fir1.c
fir2.o: fir2.c
	$(CC) $(CFLAGS) -c fir2.c
fir3.o: fir3.c
	$(CC) $(CFLAGS) -c fir3.c
proc_file.o: proc_file.c fir1.h
	$(CC) $(CFLAGS) -c proc_file.c
quad_mod.o: quad_mod.c
	$(CC) $(CFLAGS) -c quad_mod.c
bit_sync.o: bit_sync.c
	$(CC) $(CFLAGS) -c bit_sync.c
bit_det.o: bit_det.c
	$(CC) $(CFLAGS) -c bit_det.c
navtex_bytes_sm.o: navtex_bytes_sm.c
	$(CC) $(CFLAGS) -c navtex_bytes_sm.c
