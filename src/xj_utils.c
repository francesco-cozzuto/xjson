
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
		
		for(size_t i = 0; i < 8; i++)
			if(v1.as_small_string[i] != v2.as_small_string[i])
				return 0;
		break;

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

void xj_dump(xj_type_t type, xj_generic_t value, FILE *f, int file_is_small)
{
	switch(type) {

		case xj_INT: 	fprintf(f, "%lld", value.as_int);
					 	break;

		case xj_NULL: 	fprintf(f, "null"); 
					  	break;
		
		case xj_TRUE:	fprintf(f, "true");
						break;

		case xj_FALSE:	fprintf(f, "false");
						break;

		case xj_FLOAT: 	fprintf(f, "%f", value.as_float);
						break;

		case xj_ARRAY:break;

		case xj_STRING:	fprintf(f, "%s", value.as_string->content);
						break;

		case xj_OBJECT:break;

		case xj_INT 	| xj_UNPARSED:	
		case xj_FLOAT 	| xj_UNPARSED:
		case xj_STRING 	| xj_UNPARSED:	if(file_is_small)
											fprintf(f, "<from %d, long %d>", value.as_unparsed.offset, value.as_unparsed.length);
										else
											fprintf(f, "<from %d, long %d>", value.as_unparsed_2->offset, value.as_unparsed_2->length);
										break;

		case xj_OBJECT 	| xj_IS_SMALL:break;
		case xj_STRING 	| xj_IS_SMALL:break;
		default:
		assert(0);
	}
}