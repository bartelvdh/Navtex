CC=gcc
###CFLAGS=-O -Wall
CFLAGS= -O3 -Wall -Wl,-rpath=/usr/local/lib64
LDLIBS=-lsdrplay_api  -lcjson -pthread -ldl

all: dumpiqw  nav_sched proc_file create_test_wav

clean:
	rm -f *.o  dumpiqw makepng nav
dumpiqw.o: dumpiqw.c
	$(CC) $(CFLAGS) -c dumpiqw.c
create_test_wav.o: create_test_wav.c
	$(CC) $(CFLAGS) -c create_test_wav.c
dumpiqw: dumpiqw.o wav.o
	$(CC) $(CFLAGS) dumpiqw.o wav.o -o dumpiqw -lm $(LDLIBS)
nav_sched.o: nav_sched.c fir1.h
	$(CC) $(CFLAGS) -c nav_sched.c
nav_sched: capt_sched.o nav_sched.o fir1.o fir2.o fir3.o fir4.o quad_mod.o bit_sync.o bit_det.o navtex_bytes_sm.o message_store.o sqlite3.o wav.o
	$(CC) $(CFLAGS) capt_sched.o nav_sched.o fir1.o fir2.o fir3.o fir4.o quad_mod.o bit_sync.o bit_det.o navtex_bytes_sm.o  message_store.o wav.o sqlite3.o -o nav_sched -lm $(LDLIBS)
proc_file: proc_file.o nav_sched.o fir1.o fir2.o fir3.o fir4.o quad_mod.o bit_sync.o bit_det.o navtex_bytes_sm.o message_store.o sqlite3.o wav.o
	$(CC) $(CFLAGS) proc_file.o nav_sched.o fir1.o fir2.o fir3.o fir4.o quad_mod.o bit_sync.o bit_det.o navtex_bytes_sm.o  message_store.o wav.o sqlite3.o -o proc_file -lm $(LDLIBS)
create_test_wav: create_test_wav.o wav.o
	$(CC) $(CFLAGS)  wav.o create_test_wav.o -o create_test_wav -lm $(LDLIBS)
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
fir4.o: fir4.c
	$(CC) $(CFLAGS) -c fir4.c
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
file_purger.o: file_purger.c
	$(CC) $(CFLAGS) -c file_purger.c
wav.o: wav.c
	$(CC) $(CFLAGS) -c wav.c
