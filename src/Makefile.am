## This Source Code Form is subject to the terms of the Mozilla Public
## License, v. 2.0. If a copy of the MPL was not distributed with this
## file, You can obtain one at https://mozilla.org/MPL/2.0/.

## Copyright (c) 2016 David Jackson

AM_CFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)

bin_PROGRAMS = cyto cymkd
cyto_SOURCES = main.c layout.c layout.h cytogen_header.c cytogen_header.h \
			   string_util.c string_util.h files.c files.h processing.c \
			   processing.h render.c render.h common.h cyto_config.c \
			   cyto_config.h feed.c feed.h
cyto_LDADD = $(top_srcdir)/lib/libcymkd.la -lpthread \
			 $(top_srcdir)/lib/libcyjson.a

cymkd_SOURCES = cymkd.c
cymkd_LDADD = $(top_srcdir)/lib/libcymkd.la
