
#include <string.h>
#include <time.h>
#include "xj.h"

xj_generic_t the_empty_array = { .as_array = &(xj_array_t) { .size = 0, .types = NULL, .values = NULL } };
xj_generic_t the_empty_object = { .as_small_object = &(xj_small_object_t) { .size = 0, .map = NULL, .map_size = 0, .item_types = NULL, .item_values = NULL } };

#if xj_MONITOR_PARSER

static inline char _next(xj_context_t *ctx, const char *func, size_t line)
{
	char c;

	if(ctx->offset == ctx->length)
		c = '\0';

	c = ctx->source[ctx->offset++];

	switch(c) {
		case '\0': printf("%s:%-6ld\t:: \\0\n", func, line); break;
		case '\n': printf("%s:%-6ld\t:: \\n\n", func, line); break;
		case '\t': printf("%s:%-6ld\t:: \\t\n", func, line); break;
		case ' ': printf("%s:%-6ld\t:: (space)\n", func, line); break;
		default  : printf("%s:%-6ld\t:: %c\n", func, line, c); break;
	}

	return c;
}

static inline char _current(xj_context_t *ctx, const char *func, size_t line)
{
	(void) func;
	(void) line;

	return ctx->source[ctx->offset-1];
}

static inline void _back(xj_context_t *ctx, const char *func, size_t line)
{
	(void) func;
	(void) line;

	ctx->offset--;
}

#define next(ctx) _next(ctx, __func__, __LINE__)
#define back(ctx) _back(ctx, __func__, __LINE__)
#define current(ctx) _current(ctx, __func__, __LINE__)

#else

static inline char next(xj_context_t *ctx)
{
	if(ctx->offset == ctx->length)
		return '\0';

	return ctx->source[ctx->offset++];
}

static inline char current(xj_context_t *ctx)
{
	return ctx->source[ctx->offset-1];
}

static inline void back(xj_context_t *ctx)
{
	ctx->offset--;
}

#endif

static inline size_t here(xj_context_t *ctx)
{
	return ctx->offset-1;
}

static inline int can_be_lazy(xj_context_t *ctx)
{
	return ctx->flags & xj_ALLOW_LAZYNESS;
}

static inline int is_whitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\n';
}

static inline int is_digit(char c)
{
	return c <= '9' && c >= '0';
}

static int skip_spaces(xj_context_t *ctx)
{
	char c;

	while(is_whitespace(c = next(ctx)));

	if(c == '\0')
		return 0;

	back(ctx);

	return 1;
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

static int parse_value(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value);

static int parse_2(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value)
{
	if(!skip_spaces(ctx)) {

		xj_report(ctx, "The source is empty");
		return 0;
	}

	if(!parse_value(ctx, type, value)) {

		// The error was already reported.
		return 0;
	}
	
	if(skip_spaces(ctx)) {

		xj_report(ctx, "The source contains something other than the root value");
		return 0;
	}
	

	return 1;
}

int xj_parse_2(const char *source, size_t length, char *error_buffer, size_t error_buffer_size, size_t *error_offset, size_t *error_column, size_t *error_lineno, int flags, xj_item_t *item, xj_pool_t *pool)
{
	// Set default values for output arguments

	if(error_offset) *error_offset = 0;
	if(error_column) *error_column = 1;
	if(error_lineno) *error_lineno = 1;

	// Check for argument errors

	if(pool == NULL) {

		snprintf(error_buffer, error_buffer_size, "The pool pointer can't be null");
		return 0;
	}

	if(source == NULL) {

		snprintf(error_buffer, error_buffer_size, "The source pointer can't be null");
		return 0;
	}

	if(length == 0) {

		snprintf(error_buffer, error_buffer_size, "The source is empty");
		return 0;
	}

	xj_context_t ctx;
	ctx.flags = flags;
	ctx.error_buffer = error_buffer;
	ctx.error_buffer_size = error_buffer_size;
	ctx.source = source;
	ctx.length = length;
	ctx.offset = 0;
	ctx.pool = pool;

#if xj_MONITOR_VALUE_KINDS
	ctx.empty_object_count = 0;
	ctx.empty_array_count = 0;
	ctx.small_strings_count = 0;
	ctx.normal_strings_count = 0;
	ctx.unparsed_ints_count = 0;
	ctx.unparsed_floats_count = 0;
	ctx.unparsed_strings_count = 0;
#endif

	xj_stack_setup(&ctx.stack);
	xj_pool_setup(pool);

	xj_type_t type;
	xj_generic_t value;

	if(!parse_2(&ctx, &type, &value)) {

		// The error was already reported.

		if(error_offset) *error_offset = ctx.offset;
		get_position_from_offset(ctx.source, ctx.offset, error_column, error_lineno);

		xj_pool_release(ctx.pool);
		xj_stack_free(&ctx.stack);
		return 0;
	}

	if(item) {

		item->type = type;
		item->value = value;
	}

#if xj_MONITOR_VALUE_KINDS

	snprintf(error_buffer, error_buffer_size, 
		"We good!\n"
		"empty objects ...... %ld\n"
		"empty arrays ....... %ld\n"
		"small strings ...... %ld\n"
		"normal strings ..... %ld\n"
		"unparsed ints ...... %ld\n"
		"unparsed floats .... %ld\n"
		"unparsed strings ... %ld",
		ctx.empty_object_count,
		ctx.empty_array_count,
		ctx.small_strings_count,
		ctx.normal_strings_count,
		ctx.unparsed_ints_count,
		ctx.unparsed_floats_count,
		ctx.unparsed_strings_count);

#else
	
	snprintf(error_buffer, error_buffer_size, "We good!");

#endif

	xj_stack_free(&ctx.stack);
	return 1;
}

int xj_parse(const char *source, size_t length, xj_item_t *item, xj_pool_t *pool)
{
	return xj_parse_2(source, length, NULL, 0, NULL, NULL, NULL, 0, item, pool);
}

int xj_parsefile_2(const char *path, char *error_buffer, size_t error_buffer_size, size_t *error_offset, size_t *error_column, size_t *error_lineno, int flags, xj_item_t *item, xj_pool_t *pool)
{
	// Set default values for output arguments

	if(error_offset) *error_offset = 0;
	if(error_column) *error_column = 1;
	if(error_lineno) *error_lineno = 1;

	// Do some check on the input arguments

	if(path == NULL) {

		snprintf(error_buffer, error_buffer_size, "The path can't be null");
		return 0;
	}

	// Load the file in memory

	FILE *f = fopen(path, "rb");

	if(f == NULL) {

		snprintf(error_buffer, error_buffer_size, "Failed to open \"%s\"", path);
		return 0;
	}

	fseek(f, 0, SEEK_END);

	size_t length = ftell(f);

	fseek(f, 0, SEEK_SET);

	char *source = malloc(length);

	if(source == NULL) {

		fclose(f);

		snprintf(error_buffer, error_buffer_size, "Out of memory. Couldn't allocate memory to load \"%s\"", path);
		return 0;
	}

	size_t loaded = fread(source, 1, length, f);

	if(loaded != length) {

		fclose(f);
		free(source);

		snprintf(error_buffer, error_buffer_size, "fread couldn't load all of \"%s\" in memory. Were expected %ld bytes but %ld were loaded by fread", path, length, loaded);
		return 0;
	}

	fclose(f);

	int result = xj_parse_2(source, length, error_buffer, error_buffer_size, error_offset, error_column, error_lineno, flags, item, pool);

	free(source);

	return result;
}

int xj_parsefile(const char *path, xj_item_t *item, xj_pool_t *pool)
{
	return xj_parsefile_2(path, NULL, 0, NULL, NULL, NULL, 0, item, pool);
}

static int parse_null(xj_context_t *ctx);
static int parse_true(xj_context_t *ctx);
static int parse_false(xj_context_t *ctx);
static int parse_array(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value);
static int parse_number(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value);
static int parse_object(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value);
static int parse_string(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value, int is_key);

static int create_small_string(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value);
static int create_small_object(xj_context_t *ctx, size_t item_count, xj_generic_t *value);
static int create_unparsed(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value);
static int create_string(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value);
static int create_object(xj_context_t *ctx, size_t item_count, xj_generic_t *value);
static int create_array(xj_context_t *ctx, size_t item_count, xj_generic_t *value);
static int create_float(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value);
static int create_int(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value);

static int parse_value(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value)
{
	if(!skip_spaces(ctx)) {

		xj_report(ctx, "Unexpected end of source. Was expected a value");
		return 0;
	}

	char c = next(ctx);

	back(ctx);

	int result;

	switch(c) {

		case 'n': 	if(type) *type = xj_NULL;
				   	result = parse_null(ctx);
				   	break;

		case 't': 	if(type) *type = xj_FALSE;
				   	result = parse_true(ctx);
				   	break;

		case 'f': 	if(type) *type = xj_FALSE;
				   	result = parse_false(ctx);
				   	break;

		case '[': 	result = parse_array(ctx, type, value);
					break;

		case '"': 	result = parse_string(ctx, type, value, 0);
					break;

		case '{': 	result = parse_object(ctx, type, value);
					break;
		
		default : 	if(is_digit(c)) {

				   		result = parse_number(ctx, type, value);
				   		break;
				   	}
				   	
				   	xj_report(ctx, "Unexpected character [%c]. Was expected a value", c);
				   	return 0;
	}

	return result;
}

int parse_null(xj_context_t *ctx)
{

	
	char c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source. The null value was expected");
		return 0;
	}

	if(c != 'n') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [n] since the null value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [n]. The null value was expected");
		return 0;
	}

	if(c != 'u') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [u] since the null value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [u]. The null value was expected");
		return 0;
	}

	if(c != 'l') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [l] since the null value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [l]. The null value was expected");
		return 0;
	}

	if(c != 'l') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [l] since the null value was expected", c);
		return 0;
	}

	return 1;
}

int parse_true(xj_context_t *ctx)
{

	
	char c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source. The true value was expected");
		return 0;
	}

	if(c != 't') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [t] since the true value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [t]. The true value was expected");
		return 0;
	}

	if(c != 'r') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [r] since the true value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [r]. The true value was expected");
		return 0;
	}

	if(c != 'u') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [u] since the true value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [u]. The true value was expected");
		return 0;
	}

	if(c != 'e') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [e] since the true value was expected", c);
		return 0;
	}

	return 1;
}


static int parse_false(xj_context_t *ctx)
{

	
	char c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source. The false value was expected");
		return 0;
	}

	if(c != 'f') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [f] since the false value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [f]. The false value was expected");
		return 0;
	}

	if(c != 'a') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [a] since the false value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [a]. The false value was expected");
		return 0;
	}

	if(c != 'l') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [l] since the false value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [l]. The false value was expected");
		return 0;
	}

	if(c != 's') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [s] since the false value was expected", c);
		return 0;
	}


	c = next(ctx);

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source after [s]. The false value was expected");
		return 0;
	}

	if(c != 'e') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [e] since the false value was expected", c);
		return 0;
	}

	return 1;
}

static int parse_number(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value)
{
	int is_float = 0;

	char c = next(ctx);

	if(!is_digit(c)) {

		xj_report(ctx, "Unexpected character [%c]. Was expected a digit since a numeric value was expected", c);
		return 0;
	}

	size_t offset = here(ctx);

	while(is_digit(c = next(ctx)));

	if(c == '.') {

		c = next(ctx);

		if(!is_digit(c)) {

			xj_report(ctx, "Unexpected character [%c]. Was expected a digit after [.] in float value", c);
			return 0;
		}

		while(is_digit(c = next(ctx)));

		is_float = 1;
	}

	size_t length = here(ctx) - offset;

	back(ctx);

	if(is_float) {

		if(length > xj_FLOAT_LAZY_TRESHOLD && can_be_lazy(ctx)) {

#if xj_MONITOR_VALUE_KINDS
			ctx->unparsed_floats_count++;
#endif

			if(type) *type = xj_FLOAT | xj_UNPARSED;

			if(!create_unparsed(ctx, offset, length, value))

				// Already reported.
				return 0;

		} else {

			if(type) *type = xj_FLOAT;

			if(!create_float(ctx, offset, length, value))

				// Already reported.
				return 0;

		}
	
	} else {

		if(length > xj_INT_LAZY_TRESHOLD && can_be_lazy(ctx)) {

#if xj_MONITOR_VALUE_KINDS
			ctx->unparsed_ints_count++;
#endif

			if(type) *type = xj_INT | xj_UNPARSED;

			if(!create_unparsed(ctx, offset, length, value))

				// Already reported.
				return 0;

		} else {

			if(type) *type = xj_INT;

			if(!create_int(ctx, offset, length, value))

				// Already reported.
				return 0;
		}
	}

	return 1;
}

static int parse_string(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value, int is_key)
{
	char c = next(ctx);

	if(c != '"') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [\"] since a string value was expected", c);
		return 0;
	}

	size_t offset = here(ctx) + 1;

	while((c = next(ctx)) != '\"' && c != '\0') {

		if(c == '\\') {
 
			c = next(ctx);

			switch(c) {
				case '\0': xj_report(ctx, "Unexpected end of source inside of a string");
						   return 0;

			}
		}

	}

	if(c == '\0') {

		xj_report(ctx, "Unexpected end of source while parsing a string");
		return 0;
	}

	size_t length = here(ctx) - offset;

	if(length < 8) {

#if xj_MONITOR_VALUE_KINDS
		ctx->small_strings_count++;
#endif

		if(type) *type = xj_STRING | xj_IS_SMALL;

		if(!create_small_string(ctx, offset, length, value))

			// Already reported.
			return 0;

	} else if(!is_key && length > xj_STRING_LAZY_TRESHOLD && can_be_lazy(ctx)) {

#if xj_MONITOR_VALUE_KINDS
		ctx->unparsed_strings_count++;
#endif

		if(type) *type = xj_STRING | xj_UNPARSED;

		if(!create_unparsed(ctx, offset, length, value))

			// Already reported.
			return 0;

	} else {

#if xj_MONITOR_VALUE_KINDS
		ctx->normal_strings_count++;
#endif

		if(type) *type = xj_STRING;

		if(!create_string(ctx, offset, length, value))

			// Already reported.
			return 0;
	}

	return 1;
}

static int parse_array(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value)
{
	char c = next(ctx);

	if(c != '[') {

		xj_report(ctx, "Unexpected character [%c]. Was expected [[] since an array value was expected", c);
		return 0;	
	}

	c = next(ctx);

	if(c == ']') {

#if xj_MONITOR_VALUE_KINDS
	ctx->empty_array_count++;
#endif

		if(type) *type = xj_ARRAY;
		if(value) *value = the_empty_array;		
		return 1;
	}

	back(ctx);

	size_t item_count = 0;

	while(1) {

		xj_type_t    item_type;
		xj_generic_t item_value;

		if(!parse_value(ctx, &item_type, &item_value))

			// The error was already reported.
			return 0;
		
		if(!xj_stack_push(&ctx->stack, item_type, item_value)) {

			xj_report(ctx, "Out of memory. Failed to push an array item at index %d onto the stack", item_count-1);
			return 0;
		}

		item_count++;

		if(!skip_spaces(ctx)) {

			xj_report(ctx, "Unexpected end of source after array item at index %d. Was expected [,] or []]", item_count-1);
			return 0;
		}

		c = next(ctx);

		if(c == ']')
			break;

		if(c != ',') {

			xj_report(ctx, "Unexpected character [%c] inside an array, after the item at index %d. Was expected [,] or []]", c, item_count-1);
			return 0;
		}
	}

	if(type) *type = xj_ARRAY;

	if(!create_array(ctx, item_count, value))

		// Already reported.
		return 0;

	return 1;
}

static int parse_object(xj_context_t *ctx, xj_type_t *type, xj_generic_t *value)
{

	char c = next(ctx);

	if(c != '{') {

		xj_report(ctx, "Unexpected character [%c]. Was expected an object value");
		return 0;
	}

	c = next(ctx);

	if(c == '}') {

#if xj_MONITOR_VALUE_KINDS
		ctx->empty_object_count++;
#endif

		if(type) *type = xj_OBJECT | xj_IS_SMALL;

		if(value) 
			*value = the_empty_object;
		return 1;
	}

	back(ctx);

	size_t item_count = 0;

	while(1) {

		if(!skip_spaces(ctx)) {

			xj_report(ctx, "Unexpected end of source before an object key. Was expected a string");
			return 0;
		}

		xj_type_t key_type, value_type;
		xj_generic_t key, value;

		if(!parse_string(ctx, &key_type, &key, 1))

			// Already reported.
			return 0;

		if(!skip_spaces(ctx)) {

			xj_report(ctx, "Unexpected end of source after an object key. Was expected [:] as a key-value separator");
			return 0;
		}

		c = next(ctx);

		if(c != ':') {

			xj_report(ctx, "Unexpected character [%c]. Was expected [:] after object key as a key-value separator", c);
			return 0;	
		}

		if(!parse_value(ctx, &value_type, &value))

			// Already reported.
			return 0;

		if(!xj_stack_push(&ctx->stack, key_type, key)) {

			xj_report(ctx, "Out of memory. Failed to push an object key at index %d onto the stack", item_count-1);
			return 0;
		}

		if(!xj_stack_push(&ctx->stack, value_type, value)) {

			xj_report(ctx, "Out of memory. Failed to push an object item at index %d onto the stack", item_count-1);
			return 0;
		}

		item_count++;

		c = next(ctx);

		if(c == '}')
			break;

		if(c != ',') {

			xj_report(ctx, "Unexpected character [%c]. Was expected [,] or [}] after object item", c);
			return 0;				
		}

	}

	if(item_count < 256) {

		if(type) *type = xj_OBJECT | xj_IS_SMALL;

		if(!create_small_object(ctx, item_count, value))

			// Already reported.
			return 0;

	} else {

		if(type) *type = xj_OBJECT;

		if(!create_object(ctx, item_count, value))

			// Already reported.
			return 0;
	}

	return 1;
}

static int create_unparsed(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value)
{
	if(ctx->length < (u32) ~0) {

		if(value) {

			value->as_unparsed.offset = offset;
			value->as_unparsed.length = length;
		}

	} else {

		xj_unparsed_2_t *u = xj_malloc(ctx, sizeof(xj_unparsed_2_t), 1);

		if(u == NULL)
			return 0;

		u->offset = offset;
		u->length = length;

		if(value)
			value->as_unparsed_2 = u;
	}

	return 1;
}

static int create_float(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value)
{

	if(value) {

		char buffer[1024];

		memcpy(buffer, ctx->source + offset, length);
		buffer[length] = '\0';

		value->as_float = strtod(buffer, NULL);
	}

	return 1;
}

static int create_int(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value)
{

	if(value) {

		char buffer[1024];

		memcpy(buffer, ctx->source + offset, length);
		buffer[length] = '\0';

		value->as_int = strtoll(buffer, NULL, 10);
	}

	return 1;
}

static int create_string(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value)
{
	if(value) {

		xj_string_t *string = xj_malloc(ctx, sizeof(xj_string_t) + length + 1, 1);

		if(string == NULL) {

			xj_report(ctx, "Out of memory. Failed to allocate a string object");
			return 0;
		}
		/*
		printf("allocating %ld bytes for a string [", sizeof(xj_string_t) + length + 1);
		for(size_t i = 0; i < length; i++)
			printf("%c", ctx->source[offset + i]);
		printf("] at %p\n", string);
		*/
		string->length = length;
		memcpy(string->content, ctx->source + offset, length);
		string->content[length] = '\0';

		if(value)
			value->as_string = string;
	}

	return 1;
}

static int create_small_string(xj_context_t *ctx, size_t offset, size_t length, xj_generic_t *value)
{

	if(value) {

		memcpy(value->as_small_string, ctx->source + offset, length);
		memset(value->as_small_string + length, 0, 8 - length);

	}

	return 1;
}

static int create_array(xj_context_t *ctx, size_t item_count, xj_generic_t *value)
{
	xj_array_t *array = xj_malloc(ctx, sizeof(xj_array_t), 1);

	if(array == NULL) {

		xj_report(ctx, "Out of memory. Failed to allocate array object with %d items", item_count);
		return 0;
	}

	array->size = 0;
	array->types = xj_malloc(ctx, sizeof(xj_type_t) * item_count, 1);
	array->values = xj_malloc(ctx, sizeof(xj_generic_t) * item_count, 1);

	if(array->types == NULL) {

		xj_report(ctx, "Out of memory. Failed to allocate the type array of an array with %d items", item_count);
		return 0;
	}

	if(array->values == NULL) {

		xj_report(ctx, "Out of memory. Failed to allocate the value array of an array with %d items", item_count);
		return 0;
	}


	while(array->size < item_count) {

		if(!xj_stack_pop(&ctx->stack, array->types + array->size, array->values + array->size)) {

			xj_report(ctx, "Reducing stack with size %d to an array with size %d", ctx->stack.size, item_count);
			return 0;
		}

		array->size++;
	}

	if(value)
		value->as_array = array;

	return 1;
}

static int create_small_object(xj_context_t *ctx, size_t item_count, xj_generic_t *value)
{

	if(value == NULL)
		return 1;

	xj_small_object_t *object = xj_malloc(ctx, sizeof(xj_small_object_t), 1);

	if(object == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate (small) object with %d items", item_count);
		return 0;
	}

	object->size = 0;

	object->map_size = 8;

	while(object->map_size * 2.0 < item_count * 3.0)
		object->map_size <<= 1;
	
	object->map = xj_malloc(ctx, sizeof(u8) * object->map_size, 0);

	object->key_types = xj_malloc(ctx, sizeof(xj_type_t) * item_count, 1);
	object->key_values = xj_malloc(ctx, sizeof(xj_generic_t) * item_count, 1);

	object->item_types = xj_malloc(ctx, sizeof(xj_type_t) * item_count, 1);
	object->item_values = xj_malloc(ctx, sizeof(xj_generic_t) * item_count, 1);

	if(object->map == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate (small) object's map (%d items)", item_count);
		return 0;
	}

	if(object->key_types == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate (small) object's key type array (%d items)", item_count);
		return 0;
	}

	if(object->key_values == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate (small) object's key value array (%d items)", item_count);
		return 0;
	}

	if(object->item_types == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate (small) object's type array (%d items)", item_count);
		return 0;
	}

	if(object->item_values == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate (small) object's value array (%d items)", item_count);
		return 0;
	}

	object->key_types[0] = 0;

	memset(object->map, 0, sizeof(u8) * object->map_size);

	while(object->size < item_count) {

		xj_type_t    current_key_type,  current_item_type;
		xj_generic_t current_key_value, current_item_value;

		if(!xj_stack_pop(&ctx->stack, &current_item_type, &current_item_value)) {

			xj_report(ctx, "Reducing stack with size %d to an object with size %d", ctx->stack.size, item_count);
			return 0;
		}

		if(!xj_stack_pop(&ctx->stack, &current_key_type, &current_key_value)) {

			xj_report(ctx, "Reducing stack with size %d to an object with size %d", ctx->stack.size, item_count);
			return 0;
		}

		// Insert it into the map

		{
			size_t hash, perturb, i, j, mask;

			perturb = hash = xj_hash_text(current_key_type, current_key_value);

			mask = object->map_size - 1;

			i = hash & mask;

			while(1) {

				j = object->map[i];

				if(j == 0) {

					object->map[i] = object->size + 1;
					break;
				}

				assert(object->size > j-1);
				
				xj_type_t    prev_key_type  = object->key_types[j-1];
				xj_generic_t prev_key_value = object->key_values[j-1];

				if(xj_compare_text(current_key_type, prev_key_type, current_key_value, prev_key_value)) {

					xj_report(ctx, "Duplicate key");
					return 0;
				}

				perturb >>= 5;
				i = (i * 5 + perturb + 1) & mask;
			}
		}

		object->key_types[object->size]  = current_key_type;
		object->key_values[object->size] = current_key_value;

		object->item_types[object->size]  = current_item_type;
		object->item_values[object->size] = current_item_value;

		object->size++;
	}

	if(value)
		value->as_small_object = object;

	return 1;
}

static int create_object(xj_context_t *ctx, size_t item_count, xj_generic_t *value)
{
	assert(0);

	if(value == NULL)
		return 1;

	xj_object_t *object = xj_malloc(ctx, sizeof(xj_object_t), 1);

	if(object == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate object with %d items", item_count);
		return 0;
	}

	object->size = 0;
	
	object->map_size = 8;

	while(object->map_size * 2.0 < item_count * 3.0)
		object->map_size <<= 1;

	object->map = xj_malloc(ctx, sizeof(u64) * 3.0 * item_count / 2.0, 0);

	object->key_types = xj_malloc(ctx, sizeof(xj_type_t) * item_count, 0);
	object->key_values = xj_malloc(ctx, sizeof(xj_generic_t) * item_count, 1);

	object->item_types = xj_malloc(ctx, sizeof(xj_type_t) * item_count, 0);
	object->item_values = xj_malloc(ctx, sizeof(xj_generic_t) * item_count, 1);
	
	if(object->map == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate object's map (%d items)", item_count);
		return 0;
	}

	if(object->key_types == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate object's key type array (%d items)", item_count);
		return 0;
	}

	if(object->key_values == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate object's key value array (%d items)", item_count);
		return 0;
	}

	if(object->item_types == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate object's type array (%d items)", item_count);
		return 0;
	}

	if(object->item_values == NULL) {
	
		xj_report(ctx, "Out of memory. Failed to allocate (small) object's value array (%d items)", item_count);
		return 0;
	}

	memset(object->map, 0, sizeof(u64) * object->map_size);

	while(object->size < item_count) {

		xj_type_t    current_key_type,  current_item_type;
		xj_generic_t current_key_value, current_item_value;

		if(!xj_stack_pop(&ctx->stack, &current_item_type, &current_item_value)) {

			xj_report(ctx, "Reducing stack with size %d to an object with size %d", ctx->stack.size, item_count);
			return 0;
		}

		if(!xj_stack_pop(&ctx->stack, &current_key_type, &current_key_value)) {

			xj_report(ctx, "Reducing stack with size %d to an object with size %d", ctx->stack.size, item_count);
			return 0;
		}

		// Insert it into the map

		{

			long hash, perturb, i, j, mask;

			hash = xj_hash_text(current_key_type, current_key_value);

			perturb = hash;

			mask = object->map_size - 1;

			i = hash & mask;

			while(1) {

				j = object->map[i];

				if(j == 0) {

					object->map[i] = object->size + 1;
					break;
				}

				xj_type_t    prev_key_type  = object->key_types[j-1];
				xj_generic_t prev_key_value = object->key_values[j-1];

				if(xj_compare_text(current_key_type, prev_key_type, current_key_value, prev_key_value)) {

					xj_report(ctx, "Duplicate key");
					return 0;
				}

				perturb >>= 5;
				i = (i * 5 + perturb + 1) & mask;
			}
		}

		object->key_types[object->size]  = current_key_type;
		object->key_values[object->size] = current_key_value;

		object->item_types[object->size]  = current_item_type;
		object->item_values[object->size] = current_item_value;

		object->size++;
	}

	if(value)
		value->as_object = object;

	return 1;
}