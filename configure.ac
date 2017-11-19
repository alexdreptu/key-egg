#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([key_egg], [0.2.2], [alexdreptu@gmail.com])
AC_CONFIG_SRCDIR([src/getinfo.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

CFLAGS="${CFLAGS=}"
LIBS="${LIBS=}"

# Enable debugging
AC_ARG_ENABLE(debug,
[AC_HELP_STRING([--disable-debug],[build without debug information.])],[enable_debug="$enableval"],[enable_debug="yes"])

if test "${enable_debug}" = "yes"; then
        AC_DEFINE([DEBUG],[1],[Enable extra debugging.])
	CFLAGS="$CFLAGS -g -Wall -O0"
else CFLAGS="$CFLAGS -O3 -fomit-frame-pointer"
fi

# Enable SENDMAIL notify
AC_ARG_ENABLE([sendmail],
	[AC_HELP_STRING([--disable-sendmail], [disable e-mail notifications.])], [enable_sendmail="$enableval"], [enable_sendmail="yes"])

if test ${enable_sendmail} = "yes"; then
	AC_DEFINE([_SENDMAIL], [1], [Enable E-Mail Notifications.])
fi

# Enable Pager notifications.
AC_ARG_ENABLE([pager],
	[AC_HELP_STRING([--disable-pager], [disable pager notifications.])], [enable_pager=$enableval], [enable_pager="yes"])

if test ${enable_pager} = "yes"; then
	AC_DEFINE([_PAGER], [1], [Enable Pager Notifications.])
fi


# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h \
	sys/socket.h sys/time.h unistd.h,, AC_MSG_ERROR(Some required header not found on yout system))

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS(gethostbyname gethostname inet_ntoa memset mkdir select socket uname,, \
	AC_MSG_ERROR(Some vital functions not found on your system))

AC_SUBST(CFLAGS)
AC_SUBST(LIBS)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
