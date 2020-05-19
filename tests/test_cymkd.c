/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2020 David Jackson
 */

#include "cymkd.h"
#include <stdlib.h>
#include <astrounit.h>
#include <string.h>

#define BUFSIZE 200

ASTRO_TEST_BEGIN(test_link_in_unordered_list)
{
	char content[] = "* this is a list\n* [Example](http://example.com) is a website\n* final bullet";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<ul><li>this is a list</li><li><a href=\"http://example.com\">Example</a> is a website</li><li>final bullet</li></ul>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
	printf("DONE");
}
ASTRO_TEST_END

int
main(void)
{
    int num_failures;
    struct astro_suite *suite;

    suite = astro_suite_create();
    astro_suite_add_test(suite, test_link_in_unordered_list, NULL);
    astro_suite_run(suite);
    num_failures = astro_suite_num_failures(suite);
    astro_suite_destroy(suite);

    return (num_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
