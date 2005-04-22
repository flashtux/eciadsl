# Process this file with autoconf to produce a configure script.
AC_INIT([eciadsl-usermode],[0.10],[eci@ml.free.fr])
AC_CONFIG_SRCDIR([check-hdlc-bug.c])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AM_PATH_PUSB(,,)

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([alarm gettimeofday memset select socket strcasecmp strdup strtol strtoul uname])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
