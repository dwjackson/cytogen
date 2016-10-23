## This Source Code Form is subject to the terms of the Mozilla Public
## License, v. 2.0. If a copy of the MPL was not distributed with this
## file, You can obtain one at https://mozilla.org/MPL/2.0/.

## Copyright (c) 2016 David Jackson

TESTS = test_cyjson test_cyto_config

bin_PROGRAMS = test_cyjson test_cyto_config

test_cyjson_SOURCES = test_cyjson.c
test_cyjson_CFLAGS = -g -Wall -lastrounit -I$(top_srcdir)/include
test_cyjson_LDADD = $(top_srcdir)/lib/libcyjson.la

test_cyto_config_SOURCES = test_cyto_config.c $(top_srcdir)/src/cyto_config.c
test_cyto_config_CFLAGS = -g -Wall -lastrounit -I$(top_srcdir)/include
test_cyto_config_LDADD = $(top_srcdir)/lib/libcyjson.la
