
#include <stdio.h>
#include <xjson.h>

int main()
{
	char buffer[1024];
	size_t offset, column, lineno;

	xj_pool_t pool;
	xj_type_t type;
	xj_generic_t value;

	const char path[] = "samples/sample_000.json";

	if(!xj_parsefile(path, buffer, sizeof(buffer), &offset, &column, &lineno, 0, &type, &value, &pool)) {

		fprintf(stderr, "Error in %s:%ld:%ld, offset %ld: %s\n", path, column, lineno, offset, buffer);

	} else {

		fprintf(stderr, "%s\n", buffer);

		xj_dump(type, value, stdout, 1);

		xj_pool_release(&pool);
	}

	return 0;
}