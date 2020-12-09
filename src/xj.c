
#include "xj.h"

void xj_print(xj_result_t result)
{
	if(result.failed) {

		fprintf(stderr, "Error in %ld:%ld, offset %ld: %s\n", result.column, result.lineno, result.offset, result.message);
	
	} else {

		fprintf(stdout, "%s\n", result.message);
	}
}

void xj_done(xj_result_t result)
{
	if(result.pool)
		xj_pool_release(result.pool);

	free(result.message);
}

static void get_position_from_offset(const char *source, size_t offset, size_t *_column, size_t *_lineno)
{
	size_t i = 0;
	size_t lineno = 1;
	size_t column = 1;
	
	while(i < offset) {

		if(source[i++] == '\n') {

			column = 1;
			lineno++;

		} else {

			column++;
		}
	}

	if(_column) *_column = column;
	if(_lineno) *_lineno = lineno;
}

xj_result_t xj_parse(const char *source, size_t length, int flags)
{
	xj_result_t result;
	result.failed = 0;
	result.offset = 0;
	result.column = 1;
	result.lineno = 1;
	result.pool = NULL;
	result.message = NULL;
	result.root.type = xj_NULL;

	result.message = malloc(xj_MAX_ERROR_TEXT_LENGTH);

	if(result.message == NULL) {

		result.failed = 1;
		return result;
	}

	xj_pool_t *pool = malloc(sizeof(xj_pool_t));	

	if(pool == NULL) {

		result.failed = 1;
		snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "Out of memory! Failed to allocate the pool");
		return result;
	}

	xj_pool_setup(pool);

	result.pool = pool;

	// Check for argument errors

	if(source == NULL) {

		result.failed = 1;
		snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "The source pointer can't be null");
		return result;
	}

	if(length == 0) {

		result.failed = 1;
		snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "The source is empty");
		return result;
	}

	size_t error_offset;

	if(!parse(source, length, pool, &result.root.type, &result.root.value, flags, result.message, xj_MAX_ERROR_TEXT_LENGTH, &error_offset)) {

		// The error was already reported.

		result.pool = NULL;
		result.failed = 1;
		result.offset = error_offset;
		get_position_from_offset(source, error_offset, &result.column, &result.lineno);

		xj_pool_release(pool);
		free(pool);
		return result;
	}

	return result;
}

xj_result_t xj_parse_file(const char *path, int flags)
{
	xj_result_t result;


	// Set up the result structure.

	{
		result.failed = 0;
		result.offset = 0;
		result.column = 1;
		result.lineno = 1;
		result.pool = NULL;
		result.message = NULL;
		result.root.type = xj_NULL;

		result.message = malloc(xj_MAX_ERROR_TEXT_LENGTH);

		if(result.message == NULL) {

			result.failed = 1;
			return result;
		}

		xj_pool_t *pool = malloc(sizeof(xj_pool_t));	

		if(pool == NULL) {

			result.failed = 1;
			snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "Out of memory! Failed to allocate the pool");
			return result;
		}

		xj_pool_setup(pool);

		result.pool = pool;
	}


	// Check for argument errors.

	if(path == NULL) {

		result.failed = 1;
		snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "The path can't be null");
		return result;
	}


	// Load the file contents.

	size_t length;
	char *content;

	{
		FILE *f = fopen(path, "rb");

		if(f == NULL) {

			result.failed = 1;
			snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "Failed to open \"%s\"", path);

			xj_pool_release(result.pool);
			free(result.pool);
			result.pool = NULL;

			return result;
		}

		fseek(f, 0, SEEK_END);

		length = ftell(f);

		fseek(f, 0, SEEK_SET);

		content = malloc(length);

		if(content == NULL) {

			result.failed = 1;
			snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "Out of memory. Couldn't allocate memory to load \"%s\"", path);

			fclose(f);

			xj_pool_release(result.pool);
			free(result.pool);
			result.pool = NULL;

			return result;
		}

		size_t loaded = fread(content, 1, length, f);

		if(loaded != length) {

			result.failed = 1;
			snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "fread couldn't load all of \"%s\" in memory. Were expected %ld bytes but %ld were loaded by fread", path, length, loaded);

			fclose(f);

			xj_pool_release(result.pool);
			free(result.pool);
			result.pool = NULL;

			free(content);

			return result;
		}

		fclose(f);
	
		if(length == 0) {

			result.failed = 1;
			snprintf(result.message, xj_MAX_ERROR_TEXT_LENGTH, "The source is empty");

			xj_pool_release(result.pool);
			free(result.pool);
			result.pool = NULL;

			free(content);

			return result;
		}
	}

	size_t error_offset;

	if(!parse(content, length, result.pool, &result.root.type, &result.root.value, flags, result.message, xj_MAX_ERROR_TEXT_LENGTH, &error_offset)) {

		// The error was already reported.

		result.pool = NULL;
		result.failed = 1;
		result.offset = error_offset;
		get_position_from_offset(content, error_offset, &result.column, &result.lineno);

		xj_pool_release(result.pool);
		free(result.pool);
		result.pool = NULL;

		free(content);

		return result;
	}

	free(content);

	return result;
}