# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# Copyright (c) 2016-2020 David Jackson

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cytogen], [0.6.11], [dwjackson@users.noreply.github.com])
AC_CONFIG_SRCDIR([src/cytogen_header.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])

# Checks for programs.
AC_PROG_CC
AM_PROG_AR

# Initialize libtool
LT_INIT

# Checks for libraries.
AC_CHECK_LIB([ctache], [ctache_render_string], [],
             [AC_MSG_ERROR([Could not find required library 'ctache'])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset mkdir munmap rmdir strdup basename_r])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 include/Makefile
                 src/Makefile
                 man/Makefile
                 tests/Makefile])
AC_OUTPUT
