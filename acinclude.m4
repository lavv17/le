dnl Locate ncurses or curses library
AC_DEFUN([LE_PATH_CURSES_DIRECT],
[
  ncurses_h=no
  for ac_dir in               \
    /usr/local/include/ncursesw\
    /usr/local/include/ncurses\
    /usr/local/include	      \
    /usr/include/ncursesw     \
    /usr/include/ncurses      \
    /usr/include	      \
    ; \
  do
    if test -r "$ac_dir/curses.h"; then
      no_curses=
      ac_curses_includes=$ac_dir

      if grep NCURSES_VERSION $ac_curses_includes/curses.h >/dev/null; then
	ac_with_ncurses=yes
	curses_direct_test_library=ncurses
      else
	ac_with_ncurses=no
	curses_direct_test_library=curses
      fi

      break
    fi
  done

  if test -z "$ac_curses_includes" -o "$ac_with_ncurses" = no ; then
    for ac_dir in             \
      /usr/local/include      \
      /usr/include	      \
      ; \
    do
      if test -r "$ac_dir/ncurses.h"; then
	no_curses=
	ac_curses_includes=$ac_dir
	ac_with_ncurses=yes
	ncurses_h=yes
        curses_direct_test_library=ncurses
	break
      fi
    done
  fi

# First see if replacing the include by lib works.
for ac_dir0 in `echo "$ac_curses_includes" | sed -e 's:include:lib:' -e 's:/ncurses$::'` \
    /usr/lib              \
    /usr/local/lib        \
    ; \
do
 for ac_dir in ${ac_dir0}64 ${ac_dir0}; do
  for ac_extension in so a sl; do
    if test -r $ac_dir/lib${curses_direct_test_library}w.$ac_extension; then
      use_libcursesw=yes
      no_curses= ac_curses_libraries=$ac_dir
      break 2
    fi
    if test -r $ac_dir/lib${curses_direct_test_library}.$ac_extension; then
      no_curses= ac_curses_libraries=$ac_dir
      break 2
    fi
  done
 done
done
])

AC_DEFUN([LE_PATH_CURSES],
[AC_REQUIRE_CPP()dnl

curses_includes=NONE
curses_libraries=NONE
with_ncurses=NONE
ncurses_h=NONE

AC_MSG_CHECKING(for Curses)
dnl AC_ARG_WITH(curses, [  --with-curses            enable Curses tests])
if test "x$with_curses" = xno; then
  no_curses=yes
else
  if test "x$curses_includes" != xNONE && test "x$curses_libraries" != xNONE \
        && test x$with_ncurses != xNONE && test x$ncurses_h != xNONE
  then
    no_curses=
  else
AC_CACHE_VAL(ac_cv_path_curses,
[# One or both of these vars are not set, and there is no cached value.
no_curses=yes

LE_PATH_CURSES_DIRECT

if test "$no_curses" = yes; then
  ac_cv_path_curses="no_curses=yes"
else
  ac_cv_path_curses="no_curses= ac_curses_includes=$ac_curses_includes ac_curses_libraries=$ac_curses_libraries ac_with_ncurses=$ac_with_ncurses ncurses_h=$ncurses_h use_libcursesw=$use_libcursesw"
fi])dnl
  fi
  eval "$ac_cv_path_curses"
fi # with_curses != no

if test "$no_curses" = yes; then
  AC_MSG_RESULT(no)
else
  test "x$curses_includes" = xNONE && curses_includes=$ac_curses_includes
  test "x$curses_libraries" = xNONE && curses_libraries=$ac_curses_libraries
  test "x$with_ncurses" = xNONE && with_ncurses=$ac_with_ncurses
  ac_cv_path_curses="no_curses= ac_curses_includes=$curses_includes ac_curses_libraries=$curses_libraries ac_with_ncurses=$with_ncurses"
  if test x$ac_with_ncurses = xyes
  then
    AC_MSG_RESULT([Ncurses, libraries $curses_libraries, headers $curses_includes])
  else
    AC_MSG_RESULT([libraries $curses_libraries, headers $curses_includes])
  fi
  if test x$ncurses_h = xyes; then
    AC_DEFINE([USE_NCURSES_H], 1, [define if ncurses.h instead of curses.h should be used])
  fi
fi
])

# check if mytinfo is required for ncurses usage
AC_DEFUN([LE_MYTINFO_CHECK],
[
   AC_CACHE_CHECK(whether mytinfo is required, ac_cv_need_mytinfo,
   [
      old_LIBS="$LIBS"
      old_CFLAGS="$CFLAGS"
      LIBS="$LIBS $CURSES_LIBS"
      CFLAGS="$CFLAGS $CURSES_INCLUDES"
      AC_TRY_LINK([
	 #ifdef USE_NCURSES_H
	 #include <ncurses.h>
	 #else
	 #include <curses.h>
	 #endif],[initscr();reset_prog_mode();refresh();endwin();],[ac_cv_need_mytinfo=no],
      [
	 LIBS="$LIBS -lmytinfo"
	 AC_TRY_LINK([
	    #ifdef USE_NCURSES_H
	    #include <ncurses.h>
	    #else
	    #include <curses.h>
	    #endif],[initscr();reset_prog_mode();refresh();endwin();],[ac_cv_need_mytinfo=yes],
	    [AC_MSG_ERROR(cannot make curses work)])
      ])
      LIBS="$old_LIBS"
      CFLAGS="$old_CFLAGS"
   ])
   if test x$ac_cv_need_mytinfo = xyes; then
      CURSES_LIBS="$CURSES_LIBS -lmytinfo"
   fi
])

AC_DEFUN([LE_CURSES_MOUSE],
[
   AC_LANG_SAVE
   AC_LANG_CPLUSPLUS
   AC_MSG_CHECKING(if curses provides mouse routines)
   AC_CACHE_VAL(ac_cv_curses_mouse,
   [
      old_LIBS="$LIBS"
      old_CFLAGS="$CFLAGS"
      LIBS="$LIBS $CURSES_LIBS"
      CFLAGS="$CFLAGS $CURSES_INCLUDES"
      AC_TRY_LINK([
	    #ifdef USE_NCURSES_H
	    # include <ncurses.h>
	    #else
	    # include <curses.h>
	    #endif
	 ],
	 [
	    MEVENT mev;
	    mousemask(ALL_MOUSE_EVENTS,0);
	    getmouse(&mev);
	    ungetmouse(&mev);
	 ],
	 [ac_cv_curses_mouse=yes],
	 [ac_cv_curses_mouse=no])
      LIBS="$old_LIBS"
      CFLAGS="$old_CFLAGS"
   ])
   AC_MSG_RESULT($ac_cv_curses_mouse)
   if test x$ac_cv_curses_mouse = xyes; then
      AC_DEFINE([WITH_MOUSE], 1, [define if curses provides mouse interface])
   fi
   AC_LANG_RESTORE
])

AC_DEFUN([LE_NCURSES_BUGS],
[
   if test x$ac_with_ncurses = xyes; then
      AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      AC_MSG_CHECKING(if ncurses has correct CXX_TYPE_OF_BOOL)
      AC_CACHE_VAL(ac_cv_ncurses_correct_bool,
      [
	 old_LIBS="$LIBS"
	 old_CFLAGS="$CFLAGS"
	 LIBS="$LIBS $CURSES_LIBS"
	 CFLAGS="$CFLAGS $CURSES_INCLUDES"
	 AC_TRY_RUN([
	       #ifdef USE_NCURSES_H
	       # include <ncurses.h>
	       #else
	       # include <curses.h>
	       #endif
	       int main()
	       {
	       #ifdef CXX_TYPE_OF_BOOL
		  return sizeof(CXX_TYPE_OF_BOOL)==sizeof(bool)?0:1;
	       #else
		  return 0;
	       #endif
	       }
	    ],
	    [ac_cv_ncurses_correct_bool=yes],
	    [ac_cv_ncurses_correct_bool=no],
	    [ac_cv_ncurses_correct_bool=yes])
	 LIBS="$old_LIBS"
	 CFLAGS="$old_CFLAGS"
      ])
      AC_MSG_RESULT($ac_cv_ncurses_correct_bool)
      if test x$ac_cv_ncurses_correct_bool = xno; then
	 AC_MSG_ERROR(ncurses misconfigured - wrong CXX_TYPE_OF_BOOL)
      fi
      AC_LANG_RESTORE
   fi
])

dnl determine curses' bool actual type
AC_DEFUN([LE_CURSES_BOOL],
[
   AC_MSG_CHECKING(whether curses defines bool type in C++)
   AC_CACHE_VAL(ac_cv_curses_bool_defined,
   [
      old_LIBS="$LIBS"
      old_CXXFLAGS="$CXXFLAGS"
      LIBS="$LIBS $CURSES_LIBS"
      CXXFLAGS="$CXXFLAGS $CURSES_INCLUDES"
      AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      AC_TRY_COMPILE([
	    #define bool LE_CURSES_BOOL
	    #ifdef USE_NCURSES_H
	    # include <ncurses.h>
	    #else
	    # include <curses.h>
	    #endif
	    ],
	    [LE_CURSES_BOOL var=0;],
	    [ac_cv_curses_bool_defined=yes],
	    [ac_cv_curses_bool_defined=no])
      AC_LANG_RESTORE
      LIBS="$old_LIBS"
      CXXFLAGS="$old_CXXFLAGS"
   ])
   AC_MSG_RESULT($ac_cv_curses_bool_defined)
  if test "$ac_cv_curses_bool_defined" = no; then
   AC_MSG_CHECKING(for curses bool type)
   AC_CACHE_VAL(ac_cv_curses_bool,
   [
      old_LIBS="$LIBS"
      old_CFLAGS="$CFLAGS"
      LIBS="$LIBS $CURSES_LIBS"
      CFLAGS="$CFLAGS $CURSES_INCLUDES"
      AC_TRY_RUN([
	    #ifdef USE_NCURSES_H
	    # include <ncurses.h>
	    #else
	    # include <curses.h>
	    #endif
	    int main()
	    {
	       FILE *fp = fopen("cf_test.out", "w");
	       if (fp != 0) {
		  bool x = TRUE;
		  if ((-x) >= 0)
		     fputs("unsigned ", fp);
		  if (sizeof(x) == sizeof(int))       fputs("int",  fp);
		  else if (sizeof(x) == sizeof(char)) fputs("char", fp);
		  else if (sizeof(x) == sizeof(short))fputs("short",fp);
		  else if (sizeof(x) == sizeof(long)) fputs("long", fp);
		  else fputs("unknown",fp);
		  fclose(fp);
	       }
	       return(0);
	    }
	 ],
	 [ac_cv_curses_bool="`cat cf_test.out`"
	  case "$ac_cv_curses_bool" in
	    *unknown*) ac_cv_curses_bool=unknown;;
	  esac
	 ],
	 [ac_cv_curses_bool=unknown]
	 [ac_cv_curses_bool=unknown])
      rm -f cf_test.out
      LIBS="$old_LIBS"
      CFLAGS="$old_CFLAGS"
   ])
   AC_MSG_RESULT($ac_cv_curses_bool)
   if test "x$ac_cv_curses_bool" != xunknown; then
      AC_DEFINE_UNQUOTED([LE_CURSES_BOOL_TYPE],$ac_cv_curses_bool,[define to the type used as curses internal bool type])
   fi
  fi
])

AC_DEFUN([LE_CURSES_WIDECHAR],
[
   AC_MSG_CHECKING(whether curses supports cchar_t)
   AC_CACHE_VAL(ac_cv_curses_widechar,
   [
      old_CFLAGS="$CFLAGS"
      old_LIBS="$LIBS"
      CFLAGS="$CFLAGS $CURSES_INCLUDES"
      LIBS="$LIBS $CURSES_LIBS"
      AC_TRY_LINK([
	 #define _XOPEN_SOURCE_EXTENDED
	 #ifdef USE_NCURSES_H
	 # include <ncurses.h>
	 #else
	 # include <curses.h>
	 #endif],
	 [cchar_t c;mvadd_wchnstr(0,0,&c,1);],
	 [ac_cv_curses_widechar=yes],
	 [ac_cv_curses_widechar=no])
      CFLAGS="$old_CFLAGS"
      LIBS="$old_LIBS"
   ])
   AC_MSG_RESULT($ac_cv_curses_widechar)
   if test x$ac_cv_curses_widechar = xyes; then
      AC_DEFINE([USE_MULTIBYTE_CHARS], 1, [Define to enable multibyte support])
   fi
])

dnl check if c++ compiler can use dynamic initializers for static variables
AC_DEFUN([CXX_DYNAMIC_INITIALIZERS],
[
   AC_LANG_SAVE
   AC_LANG_CPLUSPLUS
   AC_MSG_CHECKING(if c++ compiler can handle dynamic initializers)
   AC_TRY_RUN(
   [
      int f() { return 1; }
      int a=f();
      int main()
      {
	 return(1-a);
      }
   ],
   [cxx_dynamic_init=yes],
   [cxx_dynamic_init=no],
   [cxx_dynamic_init=yes])
   AC_MSG_RESULT($cxx_dynamic_init)
   if test x$cxx_dynamic_init = xno; then
      AC_MSG_ERROR(C++ compiler cannot handle dynamic initializers of static objects)
   fi
   AC_LANG_RESTORE
])

AC_DEFUN([LE_CHECK_REGEX_BUGS],[
   AC_CACHE_CHECK([for good GNU regex in libc], le_cv_good_gnu_regex,
      le_cv_good_gnu_regex=yes
      AC_TRY_RUN([
	 #include <stdio.h>
	 #include <string.h>
	 #include <regex.h>
	 int main()
	 {
	    struct re_pattern_buffer rexp_c;
	    struct re_registers regs;
	    char *rexp;
	    char *buf;
	    int bs;

	    /* check for completely broken re_search_2 in redhat-6.0(?) */

	    memset(&rexp_c,0,sizeof(rexp_c));
	    memset(&regs,0,sizeof(regs));
    
	    rexp="b";
	    buf="123abc";
	    bs=strlen(buf);

	    re_compile_pattern(rexp,strlen(rexp),&rexp_c);
	    if(re_search_2(&rexp_c,buf,bs,"",0,0,bs,0,bs)!=4)
	       return 1;


	    /* check for a segfault in default redhat-8.0 glibc (2.2.93-5) */

	    memset(&rexp_c,0,sizeof(rexp_c));
	    memset(&regs,0,sizeof(regs));
    
	    rexp="/\\\\*([[^*]]|\\\\*[[^/]])*\\\\*/";
	    buf="/*a\\nb\\nc*/\\n";
	    bs=strlen(buf);

	    re_syntax_options=RE_NO_BK_VBAR|RE_NO_BK_PARENS;
	    re_compile_pattern(rexp,strlen(rexp),&rexp_c);
	    re_search_2(&rexp_c,buf,bs,"",0,0,bs,&regs,bs);
    
	    return 0;
	 }
      ], , le_cv_good_gnu_regex=no)
   )
   if test x$le_cv_good_gnu_regex = xno; then
      am_cv_gnu_regex=no
   fi
])

# AM_WITH_REGEX
# -------------
#
# The idea is to distribute rx.[hc] and regex.[hc] together, for a
# while.  The WITH_REGEX symbol is used to decide which of regex.h or
# rx.h should be included in the application.  If `./configure
# --with-regex' is given (the default), the package will use gawk's
# regex.  If `./configure --without-regex', a check is made to see if
# rx is already installed, as with newer Linux'es.  If not found, the
# package will use the rx from the distribution.  If found, the
# package will use the system's rx which, on Linux at least, will
# result in a smaller executable file.
#
# FIXME: This macro seems quite obsolete now since rx doesn't seem to
# be maintained, while regex is.
AC_DEFUN([AM_WITH_REGEX],
[AC_LIBSOURCES([rx.h, rx.c, regex.c, regex.h])dnl
AC_MSG_CHECKING([which of GNU rx or gawk's regex is wanted])
AC_ARG_WITH(regex,
[  --without-regex         use GNU rx in lieu of gawk's regex for matching],
            [test "$withval" = yes && am_with_regex=1],
            [am_with_regex=1])
if test -n "$am_with_regex"; then
  AC_MSG_RESULT(regex)
  AC_DEFINE([WITH_REGEX], 1, [Define if using GNU regex])
  AC_CACHE_CHECK([for GNU regex in libc], am_cv_gnu_regex,
    [AC_TRY_LINK([],
                 [extern int re_max_failures; re_max_failures = 1],
		 [am_cv_gnu_regex=yes],
                 [am_cv_gnu_regex=no])])
  if test $am_cv_gnu_regex = no; then
    AC_LIBOBJ([regex])
  fi
else
  AC_MSG_RESULT(rx)
  AC_CHECK_FUNC(re_rx_search, , [AC_LIBOBJ([rx])])
fi
])
