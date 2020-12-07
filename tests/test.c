
#include <stdio.h>
#include <xjson.h>

int main()
{
	const char source[] = "{ \" }";
	size_t length = sizeof(source)-1;

	xj_result_t result = xj_parse(source, length, xj_ALLOW_LAZYNESS);

	if(result.failed) {

		fprintf(stderr, "Error in %ld:%ld, offset %ld: %s\n", result.column, result.lineno, result.offset, result.message);

	} else {

		int index = -1;
		char *key = NULL;

		while(xj_foreach(result.root, &index, &key, NULL))
			printf("Key no. %d is \"%s\"\n", index, key);

	}

	xj_done(result);
	return 0;
}