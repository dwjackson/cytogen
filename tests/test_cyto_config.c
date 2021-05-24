/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright (c) 2016-2021 David Jackson
 */

#include "cyjson.h"
#include "cyto_config.h"
#include <ctache/ctache.h>
#include <stdlib.h>
#include <astrounit.h>

ASTRO_TEST_BEGIN(test_read_config)
{
    char file_name[] = "test_config.json";
    struct cyto_config config;
    int retval = cyto_config_read(file_name, &config);
    assert_int_eq(0, retval, "Config file read failed");
    assert_str_eq("Test Site", config.title, "Wrong title");
    assert_str_eq("Test Author", config.author, "Wrong author name");
    assert_str_eq("http://example.com", config.url, "Wrong URL");
    cyto_config_destroy(&config);
}
ASTRO_TEST_END

ASTRO_TEST_BEGIN(test_convert_to_ctache_data)
{
    char file_name[] = "test_config.json";
    struct cyto_config config;
    (void) cyto_config_read(file_name, &config);
    ctache_data_t *config_data = cyto_config_to_ctache_data(&config);
    assert(config_data != NULL, "config_data is null");
    ctache_data_t *title = ctache_data_hash_table_get(config_data, "title");
    assert_str_eq("Test Site", ctache_data_string_buffer(title), "Title is wrong");
    ctache_data_t *author = ctache_data_hash_table_get(config_data, "author");
    assert_str_eq("Test Author", ctache_data_string_buffer(author), "Author is wrong");
    ctache_data_t *url = ctache_data_hash_table_get(config_data, "url");
    assert_str_eq("http://example.com", ctache_data_string_buffer(url), "URL is wrong");
    ctache_data_destroy(config_data);
    cyto_config_destroy(&config);
}
ASTRO_TEST_END

int
main(void)
{
    int num_failures;
    struct astro_suite *suite;

    suite = astro_suite_create();
    astro_suite_add_test(suite, test_read_config, NULL);
    astro_suite_add_test(suite, test_convert_to_ctache_data, NULL);
    num_failures = astro_suite_run(suite);
    astro_suite_destroy(suite);

    return (num_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
