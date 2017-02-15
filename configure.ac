#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([try], [1.0], [mjtryhorn@dblfeaturetech.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/trycmd.c])
AC_CONFIG_HEADERS([src/trycmd_config.h])
AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])

# Custom configuration.
AC_DEFINE([TRY_EYE_ARG_SUCCESS], ["-e^^"],
    [Eye string argument to use on command success.])
AC_DEFINE([TRY_EYE_ARG_FAILURE], ["-d"],
    [Eye string argument to use on command failure.])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

AC_DEFINE([DEF_SHELL_PATH], ["/bin/sh"],
    [The absolute system path to the default shell.])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([ \
    assert.h \
    getopt.h \
    libintl.h \
    locale.h \
    stdarg.h \
    stddef.h \
    stdio.h \
    stdlib.h \
    string.h \
    sys/types.h \
    sys/wait.h \
    linux/limits.h \
    unistd.h \
])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions. 
AC_FUNC_FORK
AC_CHECK_FUNCS([setlocale strchr strnlen fmemopen])

AC_OUTPUT
