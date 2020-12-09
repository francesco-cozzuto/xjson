
#include <stdio.h>
#include <xjson.h>
#include "minunit.h"

int tests_run = 0;

static char *test_empty_source() {
	
	{
		xj_result_t result = xj_parse(NULL, 0, 0);

		mu_assert("error, result.failed == 0", result.failed == 1);
		mu_assert("error, result.message == NULL", result.message != NULL);
		mu_assert("error, result.offset != 0", result.offset == 0);
		mu_assert("error, result.column != 1", result.column == 1);
		mu_assert("error, result.lineno != 1", result.lineno == 1);
		mu_assert("error, result.pool == NULL", result.pool != NULL);

		xj_done(result);
	}

	{
		xj_result_t result = xj_parse("", 0, 0);

		mu_assert("error, result.failed == 0", result.failed == 1);
		mu_assert("error, result.message == NULL", result.message != NULL);
		mu_assert("error, result.offset != 0", result.offset == 0);
		mu_assert("error, result.column != 1", result.column == 1);
		mu_assert("error, result.lineno != 1", result.lineno == 1);
		mu_assert("error, result.pool == NULL", result.pool != NULL);

		xj_done(result);
	}
	return NULL;
}

static char *test_empty_object() {

	const char sample[] = "{}";

	xj_result_t result = xj_parse(sample, sizeof(sample)-1, 0);

	mu_assert("error, result.failed == 0", result.failed == 0);
	mu_assert("error, result.message == NULL", result.message != NULL);
	mu_assert("error, result.offset != 0", result.offset == 0);
	mu_assert("error, result.column != 1", result.column == 1);
	mu_assert("error, result.lineno != 1", result.lineno == 1);
	mu_assert("error, result.pool == NULL", result.pool != NULL);

	mu_assert("error, !is_object(result.root)", xj_is_object(result.root));

	size_t size = xj_length(result.root);

	mu_assert("error, xj_length(result.root) != 0", size == 0);

	xj_done(result);
	return NULL;
}

static char *test_non_empty_and_atomic_types() {

	const char sample[] = "{ \"hello\": 1, \"wassup\": 3.54, \"this is a test\": true, \"gang\": false, \"kkjnb2\": null }";

	xj_result_t result = xj_parse(sample, sizeof(sample)-1, 0);

	if(result.failed) {

		xj_print(result);
	}

	mu_assert("error, result.failed == 0", result.failed == 0);
	mu_assert("error, result.message == NULL", result.message != NULL);
	mu_assert("error, result.offset != 0", result.offset == 0);
	mu_assert("error, result.column != 1", result.column == 1);
	mu_assert("error, result.lineno != 1", result.lineno == 1);
	mu_assert("error, result.pool == NULL", result.pool != NULL);

	mu_assert("error, !is_object(result.root)", xj_is_object(result.root));

	size_t size = xj_length(result.root);

	mu_assert("error, xj_length(result.root) != 5", size == 5);

	xj_item_t item;

	item = xj_select_by_key(result.root, "hello");
	mu_assert("error", !xj_is_undefined(item));
	mu_assert("error", xj_is_int(item));
	mu_assert("error", xj_is_number(item));

	item = xj_select_by_key(result.root, "wassup");
	mu_assert("error", !xj_is_undefined(item));
	mu_assert("error", xj_is_float(item));
	mu_assert("error", xj_is_number(item));

	item = xj_select_by_key(result.root, "this is a test");
	mu_assert("error", !xj_is_undefined(item));
	mu_assert("error", xj_is_true(item));
	mu_assert("error", xj_is_bool(item));

	item = xj_select_by_key(result.root, "gang");
	mu_assert("error", !xj_is_undefined(item));
	mu_assert("error", xj_is_false(item));
	mu_assert("error", xj_is_bool(item));

	item = xj_select_by_key(result.root, "kkjnb2");
	mu_assert("error", !xj_is_undefined(item));
	mu_assert("error", xj_is_null(item));

	item = xj_select_by_key(result.root, "give me some honey");
	mu_assert("error", xj_is_undefined(item));

	xj_done(result);
	return NULL;
}

static char *all_tests() {

	mu_run_test(test_empty_source);
	mu_run_test(test_empty_object);
	mu_run_test(test_non_empty_and_atomic_types);
	return NULL;
}

int main() {

	char *result = all_tests();

	if (result != NULL) {

		printf("%s\n", result);

	} else {

		printf("ALL TESTS PASSED\n");
	}
	
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}