#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(eciadsl-usermode, 0.12cvs, eci@ml.free.fr)
AM_INIT_AUTOMAKE([eciadsl-usermode],[0.12cvs])
AC_CONFIG_SRCDIR([eciadsl-check-hdlc-bug.c])

# if no --sysconfdir is specified, default to /etc
# --sysconfdir='${prefix}/etc' is considered the system default
test "x$sysconfdir" = 'x${prefix}/etc' && sysconfdir="/etc"

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_create)

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([alarm gettimeofday memset select socket strcasecmp strtol strtoul uname])

AC_SUBST(pppdpath)

#do it twice, just to hide bad messages, not a problem
which pppd > /dev/null 2>&1
res=$?
if test $res -eq 0 ; then
        pppdpath=`which pppd`
        echo checking for pppd... $pppdpath
else
        echo checking for pppd...NOT FOUND defaulting to \"pppd\"
        pppdpath="pppd"
fi

AC_CONFIG_FILES([VERSION eciadsl-usermode.spec])
AC_OUTPUT([Makefile
           eci-common/Makefile])
