dnl $Id$
dnl Macro to configure paths for libpusb
dnl 
dnl Put AM_PATH_PUSB(,,) into your configure.ac file and
dnl run autoconf to produce a configure script

AC_DEFUN(AM_PATH_PUSB,
[
AC_ARG_WITH(pusb,[  --with-pusb=PFX   Prefix where libpusb is installed (optional)], pusb_prefix="$withval", pusb_prefix="")
AC_ARG_WITH(pusb-libraries,[  --with-pusb-libraries=DIR   Directory where libpusb library is installed (optional)], pusb_libraries="$withval", pusb_libraries="")
AC_ARG_WITH(pusb-includes,[  --with-pusb-includes=DIR   Directory where libpusb header files are installed (optional)], pusb_includes="$withval", pusb_includes="")

  if test "x$pusb_libraries" != "x" ; then
    PUSB_LIBS="-L$pusb_libraries"
  elif test "x$pusb_prefix" != "x" ; then
    PUSB_LIBS="-L$pusb_prefix/lib"
  elif test "x$prefix" != "xNONE" ; then
    PUSB_LIBS="-L$prefix/lib"
  elif test "x$ac_default_prefix" != "xNONE" ; then
    PUSB_LIBS="-L$ac_default_prefix/lib"
  fi

  PUSB_LIBS="$PUSB_LIBS -lpusb"

  if test "x$pusb_includes" != "x" ; then
    PUSB_CFLAGS="-I$pusb_includes"
  elif test "x$pusb_prefix" != "x" ; then
    PUSB_CFLAGS="-I$pusb_prefix/include"
  elif test "x$prefix" != "xNONE" ; then
    PUSB_CFLAGS="-I$prefix/include"
  elif test "x$ac_default_prefix" != "xNONE" ; then
    PUSB_CFLAGS="-I$ac_default_prefix/include"
  fi

  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_CFLAGS="$CFLAGS"
  ac_save_LIBS="$LIBS"

  CPPFLAGS="$CPPFLAGS $PUSB_CFLAGS"
  CFLAGS="$CFLAGS $PUSB_CFLAGS"
  LIBS="$LIBS $PUSB_LIBS"

AC_CHECK_HEADER(pusb.h, AC_SUBST(PUSB_CFLAGS), AC_MSG_ERROR([
*** libpusb headers file are missing]))
AC_CHECK_LIB(pusb, pusb_endpoint_open, AC_SUBST(PUSB_LIBS), AC_MSG_ERROR([
*** libpusb library is missing]))

  CPPFLAGS="$ac_save_CPPFLAGS"
  CFLAGS="$ac_save_CFLAGS"
  LIBS="$ac_save_LIBS"
])
