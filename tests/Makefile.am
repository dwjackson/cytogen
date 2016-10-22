## This Source Code Form is subject to the terms of the Mozilla Public
## License, v. 2.0. If a copy of the MPL was not distributed with this
## file, You can obtain one at https://mozilla.org/MPL/2.0/.

## Copyright (c) 2016 David Jackson

AM_CFLAGS = -I$(top_srcdir)/include

TESTS = test_cyjson

bin_PROGRAMS = test_cyjson

test_cyjson_SOURCES = test_cyjson.c
test_cyjson_CFLAGS = -g -Wall -lastrounit
test_cyjson_LDDADD = $(top_srcdir)/lib/libcyjson.la
