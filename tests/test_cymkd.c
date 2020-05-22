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

ASTRO_TEST_BEGIN(test_paragraph)
{
	char content[] = "This is\na paragraph\nof text.";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<p>This is\na paragraph\nof text.</p>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_paragraphs)
{
	char content[] = "This is a paragraph.\n\nThis is another.\n";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<p>This is a paragraph.</p><p>This is another.</p>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END

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
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<ul><li>this is a list</li><li><a href=\"http://example.com\">Example</a> is a website</li><li>final bullet</li></ul>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_mdash)
{
	char content[] = "This sentence has an em dash--e.g. right there - not there.";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<p>This sentence has an em dash&mdash;e.g. right there - not there.</p>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_ordered_list)
{
	char content[] = "1. This is\n2. a numbered\n3. list\n123 This is not.";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<ol><li>This is</li><li>a numbered</li><li>list</li></ol><p>123 This is not.</p>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_img)
{
	char content[] = "![alt text](/some/image.png)";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<p><img src=\"/some/image.png\" alt=\"alt text\" /></p>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_img_within_headers)
{
	char content[] = "# Header 1\n\n![alt text](/some/image.png)\n\n# Header 2";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<h1>Header 1</h1><p><img src=\"/some/image.png\" alt=\"alt text\" /></p><h1>Header 2</h1>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_img_within_headers2)
{
	char content[] = "# Header 1\n\n![alt text](/some/image.png)\n\n# Header 2\n\nThis is some text.";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<h1>Header 1</h1><p><img src=\"/some/image.png\" alt=\"alt text\" /></p><h1>Header 2</h1><p>This is some text.</p>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_literal_html)
{
	char content[] = "<div class=\"testing\">This is a test.</div>";
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char expected[BUFSIZE] = "<div class=\"testing\">This is a test.</div>";
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
	fclose(out_fp);
}
ASTRO_TEST_END


int
main(void)
{
    int num_failures;
    struct astro_suite *suite;

    suite = astro_suite_create();
    astro_suite_add_test(suite, test_paragraph, NULL);
    astro_suite_add_test(suite, test_paragraphs, NULL);
    astro_suite_add_test(suite, test_link_in_unordered_list, NULL);
    astro_suite_add_test(suite, test_mdash, NULL);
    astro_suite_add_test(suite, test_ordered_list, NULL);
    astro_suite_add_test(suite, test_img, NULL);
    astro_suite_add_test(suite, test_img_within_headers, NULL);
    astro_suite_add_test(suite, test_img_within_headers2, NULL);
    astro_suite_add_test(suite, test_literal_html, NULL);
    astro_suite_run(suite);
    num_failures = astro_suite_num_failures(suite);
    astro_suite_destroy(suite);

    return (num_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
