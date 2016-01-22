AC_DEFUN([LFTP_PROG_CXXLINK],
[
   AC_MSG_CHECKING(how to link simple c++ programs)
   if test "$GCC" = yes -a "$GXX" = yes; then
      old_CXX="$CXX"
      CXX="$CC"
      AC_LANG_PUSH(C++)
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[char *a=new char[10];delete[] a;]])],[],[
	 old_LIBS="$LIBS"
	 LIBS="-lsupc++ $LIBS"
	 AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[char *a=new char[10];delete[] a;]])],[],[LIBS="$old_LIBS"; CXX="$old_CXX";])
	 ])
      AC_LANG_POP(C++)
   fi
   AC_MSG_RESULT(using $CXX)
])
