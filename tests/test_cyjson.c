/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016 David Jackson
 */

#include "cyjson.h"
#include <stdlib.h>
#include <astrounit.h>

ASTRO_TEST_BEGIN(test_parse_empty_object)
{
    enum cyjson_data_type data_type;
    cyjson_parser_t parser;
    int retval;

    cyjson_parser_init(&parser, "{}");

    retval = cyjson_parse(&parser);
    assert_int_eq(0, retval, "cyjson_parse() failed");
    data_type = cyjson_token_data_type(parser);
    assert_int_eq(CYJSON_OBJECT_BEGIN, data_type, "Wrong data type");

    retval = cyjson_parse(&parser);
    assert_int_eq(0, retval, "cyjson_parse() failed");
    data_type = cyjson_token_data_type(parser);
    assert_int_eq(CYJSON_OBJECT_END, data_type, "Wrong data type");
}
ASTRO_TEST_END

int
main(void)
{
    int num_failures;
    struct astro_suite *suite;

    suite = astro_suite_create();
    astro_suite_add_test(suite, test_parse_empty_object, NULL);
    astro_suite_run(suite);
    num_failures = astro_suite_num_failures(suite);
    astro_suite_destroy(suite);

    return (num_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
