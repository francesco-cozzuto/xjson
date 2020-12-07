
#include <stdio.h>
#include <xjson.h>

int main()
{
	char buffer[1024];
	size_t offset, column, lineno;

	xj_pool_t pool;
	xj_item_t item;

	const char path[] = "samples/3blue1brown.json";

	if(!xj_parsefile_2(path, buffer, sizeof(buffer), &offset, &column, &lineno, xj_ALLOW_LAZYNESS, &item, &pool)) {

		fprintf(stderr, "Error in %s:%ld:%ld, offset %ld: %s\n", path, column, lineno, offset, buffer);

	} else {

		fprintf(stderr, "%s\n", buffer);

		xj_dump(item, stdout);

		xj_free(&pool);
	}

	return 0;
}