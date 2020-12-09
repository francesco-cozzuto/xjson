
#include <stdio.h>
#include <xjson.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define mu_assert(test) do { if (!(test)) return "error, " #test " in " __FILE__ ":" STR(__LINE__); } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
extern int tests_run;

int tests_run = 0;

static char *test_empty_source() {
	
	{
		xj_result_t result = xj_parse(NULL, 0, 0);

		mu_assert(result.failed == 1);
		mu_assert(result.message != NULL);
		mu_assert(result.offset == 0);
		mu_assert(result.column == 1);
		mu_assert(result.lineno == 1);
		mu_assert(result.pool != NULL);

		xj_done(result);
	}

	{
		xj_result_t result = xj_parse("", 0, 0);

		mu_assert(result.failed == 1);
		mu_assert(result.message != NULL);
		mu_assert(result.offset == 0);
		mu_assert(result.column == 1);
		mu_assert(result.lineno == 1);
		mu_assert(result.pool != NULL);

		xj_done(result);
	}
	return NULL;
}

static char *test_empty_object() {

	const char sample[] = "{}";

	xj_result_t result = xj_parse(sample, sizeof(sample)-1, 0);

	mu_assert(result.failed == 0);
	mu_assert(result.message != NULL);
	mu_assert(result.offset == 0);
	mu_assert(result.column == 1);
	mu_assert(result.lineno == 1);
	mu_assert(result.pool != NULL);

	mu_assert(xj_is_object(result.root));

	size_t size = xj_length(result.root);

	mu_assert(size == 0);

	xj_done(result);
	return NULL;
}

static char *test_non_empty_and_atomic_types() {

	const char sample[] = "{ \"hello\": 1, \"wassup\": 3.54, \"this is a test\": true, \"gang\": false, \"kkjnb2\": null }";

	xj_result_t result = xj_parse(sample, sizeof(sample)-1, 0);

	if(result.failed) {

		xj_print(result);
	}

	mu_assert(result.failed == 0);
	mu_assert(result.message != NULL);
	mu_assert(result.offset == 0);
	mu_assert(result.column == 1);
	mu_assert(result.lineno == 1);
	mu_assert(result.pool != NULL);

	mu_assert(xj_is_object(result.root));

	size_t size = xj_length(result.root);

	mu_assert(size == 5);

	xj_item_t item;

	item = xj_select_by_key(result.root, "hello");
	mu_assert(!xj_is_undefined(item));
	mu_assert(xj_is_int(item));
	mu_assert(xj_is_number(item));

	item = xj_select_by_key(result.root, "wassup");
	mu_assert(!xj_is_undefined(item));
	mu_assert(xj_is_float(item));
	mu_assert(xj_is_number(item));

	item = xj_select_by_key(result.root, "this is a test");
	mu_assert(!xj_is_undefined(item));
	mu_assert(xj_is_true(item));
	mu_assert(xj_is_bool(item));

	item = xj_select_by_key(result.root, "gang");
	mu_assert(!xj_is_undefined(item));
	mu_assert(xj_is_false(item));
	mu_assert(xj_is_bool(item));

	item = xj_select_by_key(result.root, "kkjnb2");
	mu_assert(!xj_is_undefined(item));
	mu_assert(xj_is_null(item));

	item = xj_select_by_key(result.root, "give me some honey");
	mu_assert(xj_is_undefined(item));

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