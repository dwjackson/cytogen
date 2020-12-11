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

static void test_markdown(const char *content, const char *expected)
{
	size_t content_len = strlen(content);
	FILE* out_fp;
	char outbuf[BUFSIZE];

	memset(outbuf, 0, BUFSIZE);

	out_fp = fmemopen(outbuf, BUFSIZE, "w+");
	assert(out_fp != NULL, "fmemopen() failed");
	cymkd_render("test", content, content_len, out_fp);
	fwrite("\0", 1, 1, out_fp); /* For string compare */
	fseek(out_fp, 0, SEEK_SET);
	char actual[BUFSIZE];
	fread(actual, BUFSIZE, 1, out_fp);
	fclose(out_fp);
	assert_str_eq(expected, actual, "generated HTML is wrong");
}

ASTRO_TEST_BEGIN(test_paragraph)
{
	char content[] = "This is\na paragraph\nof text.";
	char expected[BUFSIZE] = "<p>This is\na paragraph\nof text.</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_paragraphs)
{
	char content[] = "This is a paragraph.\n\nThis is another.\n";
	char expected[BUFSIZE] = "<p>This is a paragraph.</p><p>This is another.</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_link_in_unordered_list)
{
	char content[] = "* this is a list\n* [Example](http://example.com) is a website\n* final bullet";
	char expected[BUFSIZE] = "<ul><li>this is a list</li><li><a href=\"http://example.com\">Example</a> is a website</li><li>final bullet</li></ul>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_mdash)
{
	char content[] = "This sentence has an em dash--e.g. right there - not there.";
	char expected[BUFSIZE] = "<p>This sentence has an em dash&mdash;e.g. right there - not there.</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_ordered_list)
{
	char content[] = "1. This is\n2. a numbered\n3. list\n123 This is not.";
	char expected[BUFSIZE] = "<ol><li>This is</li><li>a numbered</li><li>list</li></ol><p>123 This is not.</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_ordered_list_single_messy_numbers)
{
	char content[] = "1. This is\n1. a numbered\n3. list\n2. with weird nums";
	char expected[BUFSIZE] = "<ol><li>This is</li><li>a numbered</li><li>list</li><li>with weird nums</li></ol>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_img)
{
	char content[] = "![alt text](/some/image.png)";
	char expected[BUFSIZE] = "<p><img src=\"/some/image.png\" alt=\"alt text\" /></p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_img_within_headers)
{
	char content[] = "# Header 1\n\n![alt text](/some/image.png)\n\n# Header 2";
	char expected[BUFSIZE] = "<h1>Header 1</h1><p><img src=\"/some/image.png\" alt=\"alt text\" /></p><h1>Header 2</h1>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_img_within_headers2)
{
	char content[] = "# Header 1\n\n![alt text](/some/image.png)\n\n# Header 2\n\nThis is some text.";
	char expected[BUFSIZE] = "<h1>Header 1</h1><p><img src=\"/some/image.png\" alt=\"alt text\" /></p><h1>Header 2</h1><p>This is some text.</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_literal_html)
{
	char content[] = "<div class=\"testing\">This is a test.</div>";
	char expected[BUFSIZE] = "<div class=\"testing\">This is a test.</div>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_inline_with_less_than_symbol)
{
	char content[] = "We check `rbp<lbp/` before recursing.";
	char expected[BUFSIZE] = "<p>We check <code>rbp&lt;lbp/</code> before recursing.</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_block_quote)
{
	char content[] = "> This is a\n> block quote";
	char expected[BUFSIZE] = "<blockquote>This is a block quote</blockquote>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_block_quote_including_emphasis)
{
	char content[] = "> This is\n> a _block_ quote";
	char expected[BUFSIZE] = "<blockquote>This is a <em>block</em> quote</blockquote>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_block_quote2)
{
	char content[] = "> This is a\n> block quote\n\nTesting";
	char expected[BUFSIZE] = "<blockquote>This is a block quote</blockquote><p>Testing</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_html_comment)
{
	char content[] = "<!-- comment -->\nThis is a test.";
	char expected[BUFSIZE] = "<!-- comment -->\n<p>This is a test.</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_unicode)
{
	char content[] = "Diese Bücher sind nicht gut. 你喝茶吗";
	char expected[BUFSIZE] = "<p>Diese Bücher sind nicht gut. 你喝茶吗</p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_nested_list)
{
	char content[] = "* This is\n\t* a list\n\t* with\n* an indent";
	char expected[BUFSIZE] = "<ul><li>This is</li><ul><li>a list</li><li>with</li></ul><li>an indent</li></ul>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_nested_list_end_on_inner_list)
{
	char content[] = "* This is\n\t* a list\n\t* with an indent";
	char expected[BUFSIZE] = "<ul><li>This is</li><ul><li>a list</li><li>with an indent</li></ul></ul>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_nested_ordered_within_unordered)
{
	char content[] = "* This is\n\t1. a list\n\t2. with\n* an indent";
	char expected[BUFSIZE] = "<ul><li>This is</li><ol><li>a list</li><li>with</li></ol><li>an indent</li></ul>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_nested_unordered_within_ordered)
{
	char content[] = "1. This is\n\t* a list\n\t* with\n2. an indent";
	char expected[BUFSIZE] = "<ol><li>This is</li><ul><li>a list</li><li>with</li></ul><li>an indent</li></ol>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_lone_backtick)
{
	char content[] = "This is ` _ * a test *茶*";
	char expected[BUFSIZE] = "<p>This is ` _ * a test <em>茶</em></p>";
	test_markdown(content, expected);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_bare_emphasis)
{
	char content[] = "*test*";
	char expected[BUFSIZE] = "<p><em>test</em></p>";
	test_markdown(content, expected);
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
    astro_suite_add_test(suite, test_inline_with_less_than_symbol, NULL);
    astro_suite_add_test(suite, test_block_quote, NULL);
    astro_suite_add_test(suite, test_block_quote2, NULL);
    astro_suite_add_test(suite, test_block_quote_including_emphasis, NULL);
    astro_suite_add_test(suite, test_html_comment, NULL);
    astro_suite_add_test(suite, test_unicode, NULL);
    astro_suite_add_test(suite, test_nested_list, NULL);
    astro_suite_add_test(suite, test_nested_list_end_on_inner_list, NULL);
    astro_suite_add_test(suite, test_nested_ordered_within_unordered, NULL);
    astro_suite_add_test(suite, test_nested_unordered_within_ordered, NULL);
    astro_suite_add_test(suite, test_lone_backtick, NULL);
    astro_suite_add_test(suite, test_bare_emphasis, NULL);
    astro_suite_add_test(suite, test_ordered_list_single_messy_numbers, NULL);
    num_failures = astro_suite_run(suite);
    astro_suite_destroy(suite);

    return (num_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
