#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(flpsed, 0.3.3, Johannes.Hofmann@gmx.de)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/GsWidget.H])
AC_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

AC_LANG_CPLUSPLUS

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([dup2 setenv strchr strdup strrchr strstr])

# Check for fltk
AC_ARG_WITH([fltk-dir], AS_HELP_STRING([--with-fltk-dir=DIR], [fltk is installed in DIR]), [CPPFLAGS="-I$withval/include ${_cppflags}" LDFLAGS="-L$withval/lib $LDFLAGS"], [CPPFLAGS="-I/usr/X11R6/include ${_cppflags}" LDFLAGS="-L/usr/X11R6/lib $LDFLAGS"])
AC_CHECK_HEADERS([FL/Fl.H], [], [echo "Error: FL/Fl.H not found. Try './configure --with-fltk-dir=DIR'"; exit 1;])
AC_CHECK_LIB([fltk], [open], [], [echo "Error: libfltk.so not found. Try './configure --with-fltk-dir=DIR'"; exit 1;])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
