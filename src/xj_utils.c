
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

const char *xj_typename(xj_type_t type)
{
	switch(type) {
		case xj_INT: return "xj_INT";
		case xj_NULL: return "xj_NULL";
		case xj_TRUE: return "xj_TRUE";
		case xj_FALSE: return "xj_FALSE";
		case xj_FLOAT: return "xj_FLOAT";
		case xj_ARRAY: return "xj_ARRAY";
		case xj_STRING: return "xj_STRING";
		case xj_OBJECT: return "xj_OBJECT";

		case xj_INT 	| xj_UNPARSED: return "xj_INT | xj_UNPARSED";
		case xj_FLOAT 	| xj_UNPARSED: return "xj_FLOAT | xj_UNPARSED";
		case xj_STRING 	| xj_UNPARSED: return "xj_STRING | xj_UNPARSED";

		case xj_INT 	| xj_UNPARSED | xj_LONG_REACH: return "xj_INT | xj_UNPARSED | xj_LONG_REACH";
		case xj_FLOAT 	| xj_UNPARSED | xj_LONG_REACH: return "xj_FLOAT | xj_UNPARSED | xj_LONG_REACH";
		case xj_STRING 	| xj_UNPARSED | xj_LONG_REACH: return "xj_STRING | xj_UNPARSED | xj_LONG_REACH";

		case xj_OBJECT 	| xj_IS_SMALL: return "xj_OBJECT | xj_IS_SMALL";
		case xj_STRING 	| xj_IS_SMALL: return "xj_STRING | xj_IS_SMALL";

		default: return "???";
	}
}

long xj_hash_raw_text(const char *text, size_t length)
{
	if(length == 0)
		return 0;

	const char *p = text;
	long length2 = length;

	long x = *p << 7;

	while(--length2 >= 0)
		x = (1000003*x) ^ *p++;

	x ^= length;

	return x;
}

long xj_hash_text(xj_type_t type, xj_generic_t value)
{
	char *content;
	size_t length;

	switch(type) {

		case xj_STRING:
		length = value.as_string->length;
		content = value.as_string->content;
		break;

		case xj_STRING | xj_IS_SMALL:
		length = strlen(value.as_small_string);
		content = value.as_small_string;
		break;

		default:
		fprintf(stderr, "xj_hash_text on invalid value [%s]\n", xj_typename(type));
		assert(0);
	}

	return xj_hash_raw_text(content, length);
}

int xj_compare_text_2(xj_type_t t, xj_generic_t v, const char *text, size_t length)
{
	switch(t) {

		case xj_STRING | xj_IS_SMALL:
		return !strncmp(v.as_small_string, text, length);

		case xj_STRING:

		if(v.as_string->length != length)
			return 0;

		return !strncmp(v.as_small_string, text, length);
	
		default:
		fprintf(stderr, "xj_compare_text_2 on invalid value [%s]\n", xj_typename(t));
		assert(0);
	}
}

int xj_compare_text(xj_type_t t1, xj_type_t t2, xj_generic_t v1, xj_generic_t v2)
{
	if(t1 != t2)
		return 0;

	switch(t1) {

		case xj_STRING | xj_IS_SMALL:
		return *(u64*) v1.as_small_string == *(u64*) v2.as_small_string; // Yep. I just did.

		case xj_STRING:

		if(v1.as_string->length != v2.as_string->length)
			return 0;

		for(size_t i = 0; i < v1.as_string->length; i++)
			if(v1.as_string->content[i] != v2.as_string->content[i])
				return 0;
		break;
	
		default:
		fprintf(stderr, "xj_compare_text on invalid value [%s]\n", xj_typename(t1));
		assert(0);
	}

	return 1;
}

void xj_dump(xj_item_t item, FILE *f)
{
	switch(item.type) {

		case xj_INT: 	fprintf(f, "%lld", item.value.as_int);
					 	break;

		case xj_NULL: 	fprintf(f, "null"); 
					  	break;
		
		case xj_TRUE:	fprintf(f, "true");
						break;

		case xj_FALSE:	fprintf(f, "false");
						break;

		case xj_FLOAT: 	fprintf(f, "%f", item.value.as_float);
						break;

		case xj_ARRAY: 
		{

			fprintf(f, "[");
			for(int i = 0; i < item.value.as_array->size; i++) {

				xj_dump((xj_item_t) { item.value.as_array->types[i], item.value.as_array->values[i] }, f);
				
				if(i+1 < item.value.as_array->size)
					fprintf(f, ", ");
			}
			fprintf(f, "]");
			break;
		}

		case xj_STRING:	fprintf(f, "\"%s\"", item.value.as_string->content);
						break;

		case xj_OBJECT:
		assert(0);
		break;

		case xj_INT 	| xj_UNPARSED: fprintf(f, "<Int from %d, long %d>", item.value.as_unparsed.offset, item.value.as_unparsed.length); break;
		case xj_FLOAT 	| xj_UNPARSED: fprintf(f, "<Float from %d, long %d>", item.value.as_unparsed.offset, item.value.as_unparsed.length); break;
		case xj_STRING 	| xj_UNPARSED: fprintf(f, "<String from %d, long %d>", item.value.as_unparsed.offset, item.value.as_unparsed.length); break;

		case xj_INT 	| xj_UNPARSED | xj_LONG_REACH: fprintf(f, "<Int from %d, long %d>", item.value.as_unparsed_2->offset, item.value.as_unparsed_2->length); break;
		case xj_FLOAT 	| xj_UNPARSED | xj_LONG_REACH: fprintf(f, "<Float from %d, long %d>", item.value.as_unparsed_2->offset, item.value.as_unparsed_2->length); break;
		case xj_STRING 	| xj_UNPARSED | xj_LONG_REACH: fprintf(f, "<String from %d, long %d>", item.value.as_unparsed_2->offset, item.value.as_unparsed_2->length); break;

		case xj_OBJECT 	| xj_IS_SMALL:
		{

			fprintf(f, "{");
			for(int i = 0; i < item.value.as_small_object->size; i++) {

				xj_dump((xj_item_t) { item.value.as_small_object->key_types[i], item.value.as_small_object->key_values[i] }, f);
				fprintf(f, ": ");
				xj_dump((xj_item_t) { item.value.as_small_object->item_types[i], item.value.as_small_object->item_values[i] }, f);
			
				if(i+1 < item.value.as_small_object->size)
					fprintf(f, ", ");
			}
			fprintf(f, "}");
		}
		break;
		
		case xj_STRING 	| xj_IS_SMALL:
		fprintf(f, "\"%s\"", item.value.as_small_string);
		break;
		
		default:
		fprintf(stderr, "%s (%d)\n", xj_typename(item.type), item.type);
		assert(0);
	}
}

size_t xj_length(xj_item_t item)
{
	switch(item.type) {

		case xj_INT: return 0;
		case xj_NULL: return 0;
		case xj_TRUE: return 0;
		case xj_FALSE: return 0;
		case xj_FLOAT: return 0;
		case xj_ARRAY: return item.value.as_array->size;
		case xj_STRING:	return item.value.as_string->length;
		case xj_OBJECT: return item.value.as_object->size;

		case xj_INT 	| xj_UNPARSED: 
		case xj_FLOAT 	| xj_UNPARSED: 
		case xj_STRING 	| xj_UNPARSED: return item.value.as_unparsed.length;

		case xj_INT 	| xj_UNPARSED | xj_LONG_REACH: 
		case xj_FLOAT 	| xj_UNPARSED | xj_LONG_REACH: 
		case xj_STRING 	| xj_UNPARSED | xj_LONG_REACH: return item.value.as_unparsed_2->length;

		case xj_OBJECT 	| xj_IS_SMALL: return item.value.as_small_object->size;
		case xj_STRING 	| xj_IS_SMALL: return strlen(item.value.as_small_string);
		
		default:
		fprintf(stderr, "%s (%d)\n", xj_typename(item.type), item.type);
		assert(0);		
	}
}

const char *xj_tocstring_2(const xj_type_t type, const xj_generic_t *value)
{
	switch(type) {

		case xj_INT: return NULL;
		case xj_NULL: return NULL;
		case xj_TRUE: return NULL;
		case xj_FALSE: return NULL;
		case xj_FLOAT: return NULL;
		case xj_STRING:	return value->as_string->content;

		case xj_INT 	| xj_UNPARSED: return NULL;
		case xj_FLOAT 	| xj_UNPARSED: return NULL;
		case xj_STRING 	| xj_UNPARSED: return NULL;

		case xj_INT 	| xj_UNPARSED | xj_LONG_REACH: 
		case xj_FLOAT 	| xj_UNPARSED | xj_LONG_REACH: 
		case xj_STRING 	| xj_UNPARSED | xj_LONG_REACH: return NULL;

		case xj_STRING 	| xj_IS_SMALL: return value->as_small_string;
		
		case xj_ARRAY: return NULL;
		case xj_OBJECT: return NULL;
		case xj_OBJECT | xj_IS_SMALL: return NULL;
		
		default:
		fprintf(stderr, "%s (%d)\n", xj_typename(type), type);
		assert(0);
	}

	return NULL;
}

const char *xj_tocstring(const xj_item_t *item)
{
	return xj_tocstring_2(item->type, &item->value);
}

int xj_foreach(xj_item_t item, int *i, char **key, xj_item_t *child_item)
{
	(*i)++;

	if(*i < 0)
		return 0;

	if(key)
		*key = NULL;

	xj_type_t    child_item_type;
	xj_generic_t child_item_value;

	switch(item.type) {

		case xj_INT: return 0;
		case xj_NULL: return 0;
		case xj_TRUE: return 0;
		case xj_FALSE: return 0;
		case xj_FLOAT: return 0;
		case xj_STRING:	return 0;

		case xj_INT 	| xj_UNPARSED: 
		case xj_FLOAT 	| xj_UNPARSED: 
		case xj_STRING 	| xj_UNPARSED: return 0;

		case xj_INT 	| xj_UNPARSED | xj_LONG_REACH: 
		case xj_FLOAT 	| xj_UNPARSED | xj_LONG_REACH: 
		case xj_STRING 	| xj_UNPARSED | xj_LONG_REACH: return 0;

		case xj_STRING 	| xj_IS_SMALL: return 0;
		
		case xj_ARRAY: 

		if(*i >= item.value.as_array->size)
			return 0;

		child_item_type  = item.value.as_array->types[*i];
		child_item_value = item.value.as_array->values[*i];

		break;
		
		case xj_OBJECT: 
		{
			if(*i >= item.value.as_object->size)
				return 0;

			if(key) {

				*key = xj_tocstring_2(item.value.as_object->key_types[*i], item.value.as_object->key_values + *i);
			}
			
			child_item_type  = item.value.as_object->item_types[*i];
			child_item_value = item.value.as_object->item_values[*i];

			break;
		}

		case xj_OBJECT 	| xj_IS_SMALL: 
		{
			if(*i >= item.value.as_small_object->size)
				return 0;

			if(key) {

				*key = xj_tocstring_2(item.value.as_small_object->key_types[*i], item.value.as_small_object->key_values + *i);
			}

			child_item_type  = item.value.as_small_object->item_types[*i];
			child_item_value = item.value.as_small_object->item_values[*i];

			break;
		}
		
		default:
		fprintf(stderr, "%s (%d)\n", xj_typename(item.type), item.type);
		assert(0);
	}

	if(child_item) {
		child_item->type  = child_item_type;
		child_item->value = child_item_value;
	}

	return 1;
}

int xj_is_undefined(xj_item_t item)
{
	return item.type & xj_UNDEFINED;
}

int xj_is_object(xj_item_t item)
{
	return item.type & xj_OBJECT;
}

int xj_is_array(xj_item_t item)
{
	return item.type & xj_ARRAY;
}

int xj_is_string(xj_item_t item)
{
	return item.type & xj_STRING;
}

int xj_is_int(xj_item_t item)
{
	return item.type & xj_INT;
}

int xj_is_float(xj_item_t item)
{
	return item.type & xj_FLOAT;
}

int xj_is_null(xj_item_t item)
{
	return item.type & xj_NULL;
}

int xj_is_true(xj_item_t item)
{
	return item.type & xj_TRUE;
}

int xj_is_false(xj_item_t item)
{
	return item.type & xj_FALSE;
}

int xj_is_bool(xj_item_t item)
{
	return item.type & (xj_TRUE | xj_FALSE);
}

xj_item_t xj_select_by_index(xj_item_t item, size_t index)
{
	switch(item.type) {

		case xj_ARRAY:
		if(item.value.as_array->size <= index) {

			// Index out of range
			return (xj_item_t) { .type = xj_NULL };
		}

		return (xj_item_t) { .type = item.value.as_array->types[index], .value = item.value.as_array->values[index] };

		default:
		fprintf(stderr, "Uoops! Index selection on something that is not an array!\n");
		assert(0);
	}
}

xj_item_t xj_select_by_key_2(xj_item_t item, const char *key, size_t length)
{
	if(!xj_is_object(item)) {

		fprintf(stderr, "Uoops! Key selection on something that is not an object!\n");
		assert(0);
	}

	long hash, mask, perturb, i, j;

	hash = perturb = xj_hash_raw_text(key, length);

	if(item.type & xj_IS_SMALL) {

		mask = item.value.as_object->map_size - 1;

		i = hash & mask;

		while(1) {

			j = item.value.as_object->map[j];

			if(j == 0) {

				// The key is not contained
				return (xj_item_t) { .type = xj_UNDEFINED };
			}

			xj_type_t type = item.value.as_object->item_types[j-1];
			xj_generic_t value = item.value.as_object->item_values[j-1];

			if(xj_compare_text_2(type, value, key, length))

				// Yoooo. Found it.
		
				return (xj_item_t) { .type = type, .value = value };
			
			perturb >>= 5;
			i = (i * 5 + perturb + 1) & mask;
		}

	} else {

		mask = item.value.as_small_object->map_size - 1;

		i = hash & mask;

		while(1) {

			j = item.value.as_small_object->map[j];

			if(j == 0) {

				// The key is not contained
				return (xj_item_t) { .type = xj_UNDEFINED };
			}

			xj_type_t type = item.value.as_small_object->item_types[j-1];
			xj_generic_t value = item.value.as_small_object->item_values[j-1];

			if(xj_compare_text_2(type, value, key, length))

				// Yoooo. Found it.
		
				return (xj_item_t) { .type = type, .value = value };
			
			perturb >>= 5;
			i = (i * 5 + perturb + 1) & mask;
		}
	}

	return (xj_item_t) { .type = xj_UNDEFINED };
}

xj_item_t xj_select_by_key(xj_item_t item, const char *key)
{
	return xj_select_by_key_2(item, key, strlen(key));
}