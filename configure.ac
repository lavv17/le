dnl Process this file with autoconf to produce a configure script.
AC_INIT([le], [1.13.9], [le@uniyar.ac.ru])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_CONFIG_SRCDIR([src/edit.cc])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([lib/config.h])

AC_ARG_ENABLE(file-locks, [  --disable-file-locks    disable file locking],
   [enable_file_locks=$enableval],[enable_file_locks=yes])
if test "x$enable_file_locks" = xno; then
   AC_DEFINE([DISABLE_FILE_LOCKS], 1, [define if file locking does not work or is not desirable])
fi

test -z "$CXX"	     && DEFAULT_CXX=yes
test -z "$CFLAGS"    && DEFAULT_CFLAGS=yes
test -z "$CXXFLAGS"  && DEFAULT_CXXFLAGS=yes

dnl Checks for programs.
AC_PROG_CC
gl_EARLY
AC_PROG_CXX
CXX_DYNAMIC_INITIALIZERS
AC_PROG_LN_S
AC_PROG_RANLIB

AC_ARG_WITH(debug,
[  --with-debug            enable debug info],
[   with_debug=$withval;   ],
[   with_debug=no;	   ])

if test x$with_debug = xno; then
   if test x$DEFAULT_CFLAGS = xyes; then
      CFLAGS="`echo $CFLAGS | sed 's/-g//'`"
      if test -z "$CFLAGS"; then
	 CFLAGS=-O
      fi
   fi
   if test x$DEFAULT_CXXFLAGS = xyes; then
      CXXFLAGS="`echo $CXXFLAGS | sed 's/-g//'`"
      if test -z "$CXXFLAGS"; then
	 CXXFLAGS=-O
      fi
   fi
fi

if test x$GCC = xyes; then
   CFLAGS="$CFLAGS -Wall"
fi
if test x$GXX = xyes; then
   CXXFLAGS="$CXXFLAGS -Wall -Wwrite-strings -Woverloaded-virtual"
   if test x$DEFAULT_CXX = xyes; then
      LFTP_PROG_CXXLINK
   fi
   # save some bytes
   LFTP_CHECK_CXX_FLAGS([-fno-exceptions -fno-rtti])
   # check for -fno-implement-inline (doesn't work without -O in gcc 2.95.4; functions are never inlined)
   if test x$with_debug = xno; then
      LFTP_NOIMPLEMENTINLINE
   fi
fi

gl_INIT

dnl Checks for libraries.
AC_CHECK_LIB(m, exp)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h sys/time.h unistd.h sys/ioctl.h\
 sys/select.h sys/poll.h sys/mman.h sys/times.h sys/mount.h langinfo.h\
 wchar.h wctype.h sys/syslimits.h sys/param.h)

LE_PATH_CURSES
if test x$no_curses = xyes
then
   AC_MSG_ERROR(No curses found - aborting)
fi

CURSES_LIBS=""
CURSES_INCLUDES=""
if test x$curses_includes != x -a x$curses_includes != x/usr/include
then
   CURSES_INCLUDES="-I$curses_includes"
fi
if test x$curses_libraries != x -a x$curses_libraries != x/usr/lib
then
   CURSES_LIBS="-L$curses_libraries $CURSES_LIBS"
fi
if test x$with_ncurses = xyes
then
   CURSES_LIBS="$CURSES_LIBS -lncurses"
else
   CURSES_LIBS="$CURSES_LIBS -lcurses"
fi
if test x$use_libcursesw = xyes
then
   CURSES_LIBS="${CURSES_LIBS}w"
fi
LE_MYTINFO_CHECK
LE_CURSES_MOUSE
LE_CURSES_BOOL
dnl LE_NCURSES_BUGS
LE_CURSES_WIDECHAR

AC_SUBST(CURSES_LIBS)
AC_SUBST(CURSES_INCLUDES)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_UINTPTR_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_PROG_GCC_TRADITIONAL
AC_FUNC_STRCOLL
AC_CHECK_FUNCS(getcwd mkdir select strdup strerror strftime strtol mmap\
 ftruncate fchmod times btowc)

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile misc/Makefile doc/Makefile m4/Makefile])
AC_OUTPUT
