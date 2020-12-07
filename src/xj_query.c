
#include "xj.h"

/*
struct xj_query_data_t {
	
};

static int parse_integer(const char *query, )

static int parse_query(const char *query, char *error_buffer, size_t error_buffer_size)
{
	char c;
	size_t i = 0;

	while(1) {

		c = query[i++];

		if(c == '.') {

		} else if(c == '[') {

			c = query[i++];

			if(c == '\0') {

				snprintf(error_buffer, error_buffer_size, "Unexpected end of query");
				return 0;
			}

			if(c == '"') {

			} else if(is_digit(c)) {



			} else {

				snprintf(error_buffer, error_buffer_size, "Unexpected character [%c] inside of a square bracket selection. Was expected a string or an integer", c);
				return 0;
			}
		}
	}

	return 1;
}

int xj_query(xj_item_t item, int *error_index, char *error_buffer, size_t error_buffer_size, const char *format, ...)
{
	char buffer[4096];
	{
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);
	}


}
*/