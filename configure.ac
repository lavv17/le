dnl Process this file with autoconf to produce a configure script.
AC_INIT([le], [1.9.1], [le@uniyar.ac.ru])
AC_CONFIG_SRCDIR([src/edit.cc])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_ARG_ENABLE(file-locks, [  --disable-file-locks    disable file locking],
   [enable_file_locks=$enableval],[enable_file_locks=yes])
if test "x$enable_file_locks" = xno; then
   AC_DEFINE(DISABLE_FILE_LOCKS)
fi

test -z "$CXX"	     && DEFAULT_CXX=yes
test -z "$CFLAGS"    && DEFAULT_CFLAGS=yes
test -z "$CXXFLAGS"  && DEFAULT_CXXFLAGS=yes

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
CXX_DYNAMIC_INITIALIZERS
LFTP_NOIMPLEMENTINLINE
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
   CXXFLAGS="$CXXFLAGS -Wall"

   # save some bytes
   case "`$CXX --version`" in
   egcs*|2.8*|pgcc*)
	 CXXFLAGS="$CXXFLAGS -fno-exceptions -fno-rtti"
	 ;;
   esac
fi

if test x$DEFAULT_CXX = xyes; then
   LFTP_PROG_CXXLINK
fi

dnl Checks for libraries.
AC_CHECK_LIB(m, exp)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h sys/time.h unistd.h sys/ioctl.h\
 sys/select.h sys/poll.h sys/mman.h sys/times.h)

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
LE_MYTINFO_CHECK
LE_CURSES_MOUSE
LE_CURSES_BOOL
dnl LE_NCURSES_BUGS

AC_SUBST(CURSES_LIBS)
AC_SUBST(CURSES_INCLUDES)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
dnl AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_PROG_GCC_TRADITIONAL
AC_FUNC_STRCOLL
AC_CHECK_FUNCS(getcwd mkdir select strdup strerror strftime strtol poll mmap\
 ftruncate fchmod times)

dnl AC_REPLACE_GNU_GETOPT
AC_LIBOBJ(getopt)
AC_LIBOBJ(getopt1)

AC_FUNC_FNMATCH

#rx.c is faster for our needs, but it has subtle bugs.
#test -z "$with_regex" && with_regex=no

# probably we should test for rx bug, but this is simpler
ac_cv_func_re_rx_search=no

LE_CHECK_REGEX_BUGS

AM_WITH_REGEX
if test x$am_cv_gnu_regex = xno; then
   AC_CONFIG_LINKS([include/regex.h:lib/regex.h])
else
   AC_CONFIG_LINKS([include/rx.h:lib/rx.h])
fi
if test x$ac_cv_func_poll = xno; then
   AC_LIBOBJ(poll)
   AC_CONFIG_LINKS([include/poll.h:lib/poll.h])
fi
if test x$ac_cv_func_fnmatch_works = xno; then
   AC_LIBOBJ(fnmatch)
   AC_CONFIG_LINKS([include/fnmatch.h:lib/fnmatch.h])
fi

AC_CONFIG_LINKS([$ac_config_links_1])
AC_CONFIG_FILES([Makefile lib/Makefile include/Makefile src/Makefile misc/Makefile doc/Makefile])
AC_OUTPUT
