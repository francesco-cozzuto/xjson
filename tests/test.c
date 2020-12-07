
#include <stdio.h>
#include <xjson.h>

int main()
{
	char buffer[1024];
	size_t offset, column, lineno;

	xj_pool_t pool;
	xj_type_t type;
	xj_generic_t value;

	if(!xj_parsefile("samples/3blue1brown.json", buffer, sizeof(buffer), &offset, &column, &lineno, 0, &type, &value, &pool)) {

		fprintf(stderr, "Error at %ld:%ld, offset %ld: %s\n", column, lineno, offset, buffer);

	} else {

		fprintf(stderr, "%s\n", buffer);

		xj_dump(type, value, stdout, 1);

		xj_pool_release(&pool);
	}

	return 0;
}