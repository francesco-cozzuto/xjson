
#include "xj.h"

void xj_done(xj_result_t result)
{
	if(result.pool)
		xj_pool_release(result.pool);

	free(result.message);
}

void xj_print(xj_result_t result)
{
	if(result.failed) {

		fprintf(stderr, "Error in %ld:%ld, offset %ld: %s\n", result.column, result.lineno, result.offset, result.message);
	
	} else {

		fprintf(stdout, "%s\n", result.message);
	}
}