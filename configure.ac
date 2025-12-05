dnl Process this file with autoconf to produce a configure script.
AC_INIT([le],m4_esyscmd([build-aux/git-version-gen .tarball-version]),[le@uniyar.ac.ru])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_CONFIG_SRCDIR([src/edit.cc])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([lib/config.h])

AC_ARG_ENABLE([file-locks],
   AC_HELP_STRING([--disable-file-locks],[disable file locking]),
   [enable_file_locks=$enableval],[enable_file_locks=yes])
if test "x$enable_file_locks" = xno; then
   AC_DEFINE([DISABLE_FILE_LOCKS], 1, [define if file locking does not work or is not desirable])
fi

AC_ARG_WITH([default-eol],
  AC_HELP_STRING([--with-default-eol=EOL],[set the default EOL string (NL,CRNL,CR)]),
  [AC_DEFINE_UNQUOTED([DEFAULT_EOL], ["$withval"], [The default EOL string (NL,CRNL,CR)])])

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
AC_PATH_PROG(PERL,perl)

AC_SYS_LARGEFILE

AC_ARG_WITH([debug],
  AC_HELP_STRING([--with-debug],[enable debug info]),
[   with_debug=$withval;   ],
[   with_debug=no;	   ])

AC_ARG_WITH(profiling,
   AS_HELP_STRING([--with-profiling], [enable profiling]),
[   with_profiling=$withval;   ],
[   with_profiling=no;	   ])


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

if test x$with_profiling = xyes; then
   CFLAGS="$CFLAGS -pg"
   CXXFLAGS="$CXXFLAGS -pg"
   # profiling requires debugging, too, but don't nuke -O
#   if test x$with_debug != xyes; then
#      CFLAGS="$CFLAGS -g"
#      CXXFLAGS="$CXXFLAGS -g"
#   fi
fi

if test x$GCC = xyes; then
   CFLAGS="$CFLAGS -Wall"
fi
if test x$GXX = xyes; then
   CXXFLAGS="$CXXFLAGS -Wall -Wwrite-strings -Woverloaded-virtual"
   # save some bytes
   LFTP_CHECK_CXX_FLAGS([-fno-exceptions -fno-rtti])
   # check for -fno-implement-inline (doesn't work without -O in gcc 2.95.4; functions are never inlined)
   if test x$with_debug = xno; then
      LFTP_NOIMPLEMENTINLINE
   fi
fi

gl_INIT

if test x$ac_cv_func_wcwidth != xyes || test x$REPLACE_WCWIDTH = x1; then
   gl_LIBOBJ([wcwidth])
   AC_DEFINE([REPLACE_WCWIDTH], 1, [Define when using wcwidth replacement])
fi

# These are provided by gnulib (not if building with cmake)
AC_DEFINE([HAVE_MBSWIDTH_H], 1, [Define if there is mbswidth.h])
AC_DEFINE([HAVE_CONFIGMAKE_H], 1, [Define if there is configmake.h])

dnl Checks for libraries.
AC_CHECK_LIB(m, exp)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([sys/ioctl.h sys/mman.h sys/mount.h sys/syslimits.h sys/param.h])
AC_CHECK_HEADERS([linux/tiocl.h])

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
LE_TINFO_CHECK
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
AC_PROG_GCC_TRADITIONAL
AC_FUNC_STRCOLL
AC_CHECK_FUNCS(mmap fchmod)

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile misc/Makefile doc/Makefile m4/Makefile])
AC_CONFIG_FILES([le.spec])
AC_OUTPUT
