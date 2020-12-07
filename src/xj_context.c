
#include <stdarg.h>
#include "xj.h"

void _xj_report(xj_context_t *ctx, const char *func, size_t line, const char *format, ...)
{
	va_list args;

	va_start(args, format);

	int written = vsnprintf(ctx->error_buffer, ctx->error_buffer_size, format, args);
	
	if(ctx->flags & xj_SHOW_REPORT_LOCATIONS)
		snprintf(ctx->error_buffer + written, ctx->error_buffer_size - written, " (in %s:%ld)", func, line);
	va_end(args);
}

void *xj_malloc(xj_context_t *ctx, size_t size, int aligned)
{
	return xj_pool_request(ctx->pool, size, aligned);
}