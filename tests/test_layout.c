/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "layout.h"
#include <stdlib.h>
#include <astrounit.h>

ASTRO_TEST_BEGIN(test_recursive_layouts)
{
    int num_layouts;
    struct layout *layouts;
    layouts = get_layouts(&num_layouts);
    assert_int_eq(2, num_layouts, "Wrong number of layouts");
    char correct[] = "<!DOCTYPE><html><body> <div id=\"content\">{{>content}}</div>\n</body></html>\n";
    char *content = get_layout_content(layouts, num_layouts, "post");
    assert_str_eq(correct, content, "Wrong layout content");
    layouts_destroy(layouts, num_layouts);
}
ASTRO_TEST_END

int
main(void)
{
    int num_failures;
    struct astro_suite *suite;

    suite = astro_suite_create();
    astro_suite_add_test(suite, test_recursive_layouts, NULL);
    num_failures = astro_suite_run(suite);
    astro_suite_destroy(suite);

    return (num_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
