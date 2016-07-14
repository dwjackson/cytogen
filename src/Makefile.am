# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# Copyright (c) 2016 David Jackson

AM_CFLAGS = -I$(top_srcdir)/include 

bin_PROGRAMS = cyto cymkd
cyto_SOURCES = main.c layout.c cytoplasm_header.c string_util.c
cyto_LDADD = -lctache $(top_srcdir)/lib/libcymkd.la -lpthread

cymkd_SOURCES = cymkd.c
cymkd_LDADD = $(top_srcdir)/lib/libcymkd.la
