
#include "xj.h"

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

	if(length == 0)
		return 0;

	char *p = content;
	long length2 = length;

	long x = *p << 7;

	while(--length2 >= 0)
		x = (1000003*x) ^ *p++;

	x ^= length;

	return x;
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