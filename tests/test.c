
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

	xj_result_t result = xj_parse(sample, 0, 0);

	mu_assert("error, result.failed == 0", result.failed != 0);
	mu_assert("error, result.message == NULL", result.message != NULL);
	mu_assert("error, result.offset != 0", result.offset == 0);
	mu_assert("error, result.column != 1", result.column == 1);
	mu_assert("error, result.lineno != 1", result.lineno == 1);
	mu_assert("error, result.pool == NULL", result.pool != NULL);

	printf("[%s]\n", xj_typename(result.root.type));

	mu_assert("error, !is_object(result.root)", xj_is_object(result.root));

	size_t size = xj_length(result.root);

	mu_assert("error, xj_length(result.root) != 0", size == 0);

	xj_done(result);
	return NULL;
}

static char *all_tests() {

	mu_run_test(test_empty_source);
	mu_run_test(test_empty_object);
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