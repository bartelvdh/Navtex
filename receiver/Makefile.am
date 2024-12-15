bin_PROGRAMS = nav_sched_dual
nav_sched_dual_SOURCES = capt_sched.c  message_store.c  decoder.C  fir1cpp.C  fir2cpp.C  fir3cpp.C  nav_b_sm.C  nav_sched.C
#AM_LDFLAGS =  -L/usr/local/lib  -pthread -lsdrplay_api  -lcjson -ldl -lm -lsqlite3
AM_CFLAGS= -I/usr/local/lib  -I/usr/local/include
nav_sched_dual_LDADD =  -L/usr/local/lib -lsdrplay_api  -lcjson -ldl -lm -lsqlite3
nav_sched_dual_LDFLAGS =  -pthread 
