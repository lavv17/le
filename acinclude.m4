dnl Locate ncurses or curses library
AC_DEFUN(LE_PATH_CURSES_DIRECT,
[
  ncurses_h=no
  for ac_dir in               \
    /usr/include/ncurses      \
    /usr/local/include/ncurses\
    /usr/local/include	      \
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
for ac_dir in `echo "$ac_curses_includes" | sed -e 's:include:lib:' -e 's:/ncurses$::'` \
    /usr/lib              \
    /usr/local/lib        \
    ; \
do
  for ac_extension in a so sl; do
    if test -r $ac_dir/lib${curses_direct_test_library}.$ac_extension; then
      no_curses= ac_curses_libraries=$ac_dir
      break 2
    fi
  done
done
])

AC_DEFUN(LE_PATH_CURSES,
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
  ac_cv_path_curses="no_curses= ac_curses_includes=$ac_curses_includes ac_curses_libraries=$ac_curses_libraries ac_with_ncurses=$ac_with_ncurses ncurses_h=$ncurses_h"
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
    AC_DEFINE(USE_NCURSES_H)
  fi
fi
])

# check if mytinfo is required for ncurses usage
AC_DEFUN(LE_MYTINFO_CHECK,
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

AC_DEFUN(LE_CURSES_MOUSE,
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
      AC_DEFINE(WITH_MOUSE)
   fi
   AC_LANG_RESTORE
])

AC_DEFUN(LE_NCURSES_BUGS,
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
AC_DEFUN(LE_CURSES_BOOL,
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
      AC_DEFINE_UNQUOTED(LE_CURSES_BOOL_TYPE,$ac_cv_curses_bool)
   fi
  fi
])

dnl check if c++ compiler can use dynamic initializers for static variables
AC_DEFUN(CXX_DYNAMIC_INITIALIZERS,
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

AC_DEFUN(LFTP_PROG_CXXLINK,
[
   AC_MSG_CHECKING(how to link simple c++ programs)
   if test "$GCC" = yes -a "$GXX" = yes; then
      old_CXX="$CXX"
      CXX="$CC"
      AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      AC_TRY_LINK([],[char *a=new char[10];delete[] a;],
	 [],[CXX="$old_CXX";])
      AC_LANG_RESTORE
   fi
   AC_MSG_RESULT(using $CXX)
])

AC_DEFUN(LFTP_NOIMPLEMENTINLINE,
[
   AC_MSG_CHECKING(if -fno-implement-inlines implements virtual functions)
   flags="-fno-implement-inlines -Winline"
   AC_CACHE_VAL(lftp_cv_noimplementinline,
   [
      AC_LANG_SAVE
      AC_LANG_CPLUSPLUS
      old_CXXFLAGS="$CXXFLAGS"
      CXXFLAGS="$CXXFLAGS $flags"
      AC_TRY_LINK([
	 class aaa
	 {
	    int var;
	 public:
	    virtual void func() { var=1; }
	    aaa();
	    virtual ~aaa();
	 };
	 aaa::aaa() { var=0; }
	 aaa::~aaa() {}
	 ],[],
	 [lftp_cv_noimplementinline=yes],
	 [lftp_cv_noimplementinline=no])
      CXXFLAGS="$old_CXXFLAGS"
      AC_LANG_RESTORE
   ])
   AC_MSG_RESULT($lftp_cv_noimplementinline)
   if test x$lftp_cv_noimplementinline = xyes; then
      CXXFLAGS="$CXXFLAGS $flags"
   fi
])

AC_DEFUN([LE_CHECK_REGEX_BUGS],[
   AC_CACHE_CHECK([for good GNU regex in libc], le_cv_good_gnu_regex,
      AC_TRY_RUN([
	 #include <stdio.h>
	 #include <regex.h>
	 int main()
	 {
	    static struct re_pattern_buffer rexp;
	    re_compile_pattern("b",1,&rexp);
	    return(re_search_2(&rexp,"123abc",6,"",0,0,10,0,10)!=4);
	 }
      ], le_cv_good_gnu_regex=yes, le_cv_good_gnu_regex=no, le_cv_good_gnu_regex=yes))
   if test $le_cv_good_gnu_regex = no; then
      am_cv_gnu_regex=no
   fi
])
