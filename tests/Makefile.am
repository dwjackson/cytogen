## This Source Code Form is subject to the terms of the Mozilla Public
## License, v. 2.0. If a copy of the MPL was not distributed with this
## file, You can obtain one at https://mozilla.org/MPL/2.0/.

## Copyright (c) 2016-2020 David Jackson

TESTS = test_cyjson test_cyto_config test_layout test_cymkd

check_PROGRAMS = test_cyjson test_cyto_config test_layout test_cymkd


test_cyjson_SOURCES = test_cyjson.c
test_cyjson_CFLAGS = -g -Wall -I$(top_srcdir)/include -lastrounit
test_cyjson_LDADD = $(top_srcdir)/lib/libcyjson.a


test_cyto_config_SOURCES = test_cyto_config.c $(top_srcdir)/src/cyto_config.c \
			   $(top_srcdir)/src/cyto_config.h
test_cyto_config_CFLAGS = -g -Wall -lastrounit -I$(top_srcdir)/include \
			  -I$(top_srcdir)/src
test_cyto_config_LDADD = $(top_srcdir)/lib/libcyjson.a


test_layout_SOURCES = test_layout.c $(top_srcdir)/src/layout.c \
			  $(top_srcdir)/src/layout.h \
			  $(top_srcdir)/src/cytogen_header.c \
			  $(top_srcdir)/src/cytogen_header.h \
			  $(top_srcdir)/src/files.c $(top_srcdir)/src/files.h \
			  $(top_srcdir)/src/string_util.c \
			  $(top_srcdir)/src/string_util.h

test_layout_CFLAGS = -g -Wall -lastrounit -I$(top_srcdir)/include \
			  -I$(top_srcdir)/src
test_layout_LDADD = $(top_srcdir)/lib/libcyjson.a


test_cymkd_SOURCES = test_cymkd.c $(top_srcdir)/lib/cymkd.c \
		     $(top_srcdir)/include/cymkd.h
test_cymkd_CFLAGS = -g -Wall -lastrounit -I$(top_srcdir)/include \
		     -I$(top_srcdir)/src
test_cymkd_LDADD = $(top_srcdir)/lib/libcymkd.la
