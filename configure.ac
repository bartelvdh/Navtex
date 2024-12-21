AC_INIT([navtex] ,[0.01])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CXX
AC_PROG_CC
AC_CHECK_LIB(cjson,cJSON_CreateArray)
AC_CHECK_LIB(dl,dlopen)
AC_CHECK_LIB(sqlite3,sqlite3_open)
AC_CHECK_LIB(microhttpd,MHD_start_daemon)
AC_CHECK_LIB(m,atan2)
AC_CHECK_LIB(sdrplay_api,sdrplay_api_Open)
AC_CONFIG_FILES([Makefile receiver/Makefile web_server/Makefile])
AC_OUTPUT

