
enum {
	xj_INT 		= 1 << 0,
	xj_NULL 	= 1 << 1,
	xj_TRUE 	= 1 << 2,
	xj_FALSE	= 1 << 3,
	xj_FLOAT 	= 1 << 4,
	xj_ARRAY 	= 1 << 5,
	xj_STRING 	= 1 << 6,
	xj_OBJECT 	= 1 << 7,
	xj_UNDEFINED = 1 << 8,

	xj_UNPARSED = 1 << 9, // Int, float, string
	xj_IS_SMALL = 1 << 10, // Object, string
	xj_LONG_REACH = 1 << 11, // Stuff with "xj_UNPARSED"
};

enum {
	xj_ALLOW_LAZYNESS 		 = 1 << 0,
	xj_SHOW_REPORT_LOCATIONS = 1 << 1,
};

typedef struct {
	unsigned short type;
	union {
		long long as_int;
		double as_float;
		void *as_pointer;
	};
} xj_item_t;

typedef struct {
	int failed;
	void *pool;
	xj_item_t  root;
	char  *message;
	size_t offset;
	size_t column;
	size_t lineno;
} xj_result_t;

xj_result_t xj_parse(const char *source, size_t length, int flags);
xj_result_t xj_parse_file(const char *path, int flags);
void   		xj_print_message(xj_result_t result);
void   		xj_done(xj_result_t result);

size_t 		xj_length(xj_item_t item);
int    		xj_foreach(xj_item_t item, int *i, char **key, xj_item_t *child_item);

int xj_is_int(xj_item_t item);
int xj_is_null(xj_item_t item);
int xj_is_bool(xj_item_t item);
int xj_is_true(xj_item_t item);
int xj_is_false(xj_item_t item);
int xj_is_float(xj_item_t item);
int xj_is_array(xj_item_t item);
int xj_is_object(xj_item_t item);
int xj_is_string(xj_item_t item);
int xj_is_undefined(xj_item_t item);

xj_item_t xj_select_by_index(xj_item_t item, size_t index);
xj_item_t xj_select_by_key(xj_item_t item, const char *key);
xj_item_t xj_select_by_key_2(xj_item_t item, const char *key, size_t length);

const char *xj_typename(unsigned short type);