


/* ========================== */
/* === EXTERNAL UTILITIES === */
/* ========================== */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef long long i64;
typedef double    f64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned short u32;
typedef unsigned short u64;

/* ==================== */
/* === OBJECT STUFF === */
/* ==================== */

typedef u16 xj_type_t;
typedef char   xj_small_string_t[8];
typedef union  xj_generic_t xj_generic_t;
typedef struct xj_small_object_t xj_small_object_t;
typedef struct xj_object_t xj_object_t;
typedef struct xj_array_t xj_array_t;
typedef struct xj_string_t xj_string_t;
typedef struct xj_unparsed_t   xj_unparsed_t;
typedef struct xj_unparsed_2_t xj_unparsed_2_t;

struct xj_unparsed_t {

	u32 offset;
	u32 length;
};

struct xj_unparsed_2_t {

	u64 offset;
	u64 length;
};

struct xj_small_object_t {

	u8 *map;

	u16 map_size;
	u8 size;

	xj_type_t 	 *key_types;
	xj_generic_t *key_values;

	xj_type_t    *item_types;
	xj_generic_t *item_values;
};

struct xj_object_t {

	u64 *map;

	u64 map_size;
	u64 size;

	xj_type_t 	 *key_types;
	xj_generic_t *key_values;
	
	xj_type_t    *item_types;
	xj_generic_t *item_values;
};

struct xj_array_t {

	u64 size;
	xj_type_t    *types;
	xj_generic_t *values;
};

struct xj_string_t {
	size_t length;
	char   content[];
};

union xj_generic_t {

	i64 	    as_int;
	f64 	    as_float;
	xj_array_t  *as_array;
	xj_string_t *as_string;
	xj_object_t *as_object;
	xj_unparsed_t as_unparsed;
	xj_unparsed_2_t *as_unparsed_2;
	xj_small_object_t *as_small_object;
	xj_small_string_t as_small_string;
};

enum {
	
	xj_INT 		= 1 << 0,
	xj_NULL 	= 1 << 1,
	xj_TRUE 	= 1 << 2,
	xj_FALSE	= 1 << 3,
	xj_FLOAT 	= 1 << 4,
	xj_ARRAY 	= 1 << 5,
	xj_STRING 	= 1 << 6,
	xj_OBJECT 	= 1 << 7,

	xj_UNPARSED = 1 << 8, // Int, float, string
	xj_IS_SMALL = 1 << 9, // Object, string, int, float

	xj_UNPARSED_INT 	= xj_INT 	| xj_UNPARSED,
	xj_UNPARSED_FLOAT 	= xj_FLOAT 	| xj_UNPARSED,
	xj_UNPARSED_STRING 	= xj_STRING | xj_UNPARSED,
	xj_SMALL_OBJECT 	= xj_OBJECT | xj_IS_SMALL,
	xj_SMALL_STRING 	= xj_STRING | xj_IS_SMALL,
};

/* =================== */
/* === STACK STUFF === */
/* =================== */

#define xj_STACK_CHUNK_ITEM_COUNT 1024

typedef struct xj_stack_chunk_t xj_stack_chunk_t;

struct xj_stack_chunk_t {

	xj_stack_chunk_t *prev, *next;
	xj_type_t    types [xj_STACK_CHUNK_ITEM_COUNT];
	xj_generic_t values[xj_STACK_CHUNK_ITEM_COUNT];
};

typedef struct {

	xj_stack_chunk_t head, *tail;
	size_t tail_used;
	size_t size;
} xj_stack_t;

void xj_stack_setup(xj_stack_t *stack);
void xj_stack_free(xj_stack_t *stack);
int  xj_stack_push(xj_stack_t *stack, xj_type_t type, xj_generic_t value);
int  xj_stack_pop(xj_stack_t *stack, xj_type_t *type, xj_generic_t *value);

/* ================== */
/* === POOL STUFF === */
/* ================== */

#define xj_POOL_CHUNK_SIZE_IN_BYTES 65536

typedef struct xj_pool_chunk_t xj_pool_chunk_t;

struct xj_pool_chunk_t {

	xj_pool_chunk_t *next;
	size_t size;
	char body[];
};

typedef struct xj_pool_t xj_pool_t;
struct xj_pool_t {
	xj_pool_chunk_t *tail;
	size_t 			 tail_used;
	
	xj_pool_chunk_t head;
	char buffer[xj_POOL_CHUNK_SIZE_IN_BYTES]; // This is the char body[] of the previous field.
};

void  xj_pool_setup(xj_pool_t *pool);
void  xj_pool_release(xj_pool_t *pool);
void *xj_pool_request(xj_pool_t *pool, size_t size, int aligned);

/* ===================== */
/* === CONTEXT STUFF === */
/* ===================== */

#define xj_INT_LAZY_TRESHOLD 8
#define xj_FLOAT_LAZY_TRESHOLD 8
#define xj_STRING_LAZY_TRESHOLD 32

#ifndef xj_MONITOR_PARSER
#define xj_MONITOR_PARSER 0
#endif

typedef struct {

	int flags;

	char *error_buffer;
	size_t error_buffer_size;

	const char *source;
	size_t 		length;
	size_t 		offset;

	xj_stack_t stack;
	xj_pool_t *pool;

} xj_context_t;

enum {
	xj_ALLOW_LAZYNESS = 1,
	xj_SHOW_REPORT_LOCATIONS = 2,
};

void *xj_malloc(xj_context_t *ctx, size_t size, int aligned);
void _xj_report(xj_context_t *ctx, const char *func, size_t line, const char *format, ...);
#define xj_report(ctx, format, ...) _xj_report(ctx, __func__, __LINE__, format, ##__VA_ARGS__)

/* ================= */
/* === UTILITIES === */
/* ================= */

long xj_hash_text(xj_type_t type, xj_generic_t value);
int  xj_compare_text(xj_type_t t1, xj_type_t t2, xj_generic_t v1, xj_generic_t v2);


/* ====================== */
/* === EXPORTED STUFF === */
/* ====================== */

typedef struct {

	xj_type_t type;
	xj_generic_t value;
} xj_item_t;

int xj_parse(const char *source, size_t length, xj_item_t *item, xj_pool_t *pool);
int xj_parse_2(const char *source, size_t length, char *error_buffer, size_t error_buffer_size, size_t *error_offset, size_t *error_column, size_t *error_lineno, int flags, xj_item_t *item, xj_pool_t *pool);
int xj_parsefile(const char *path, xj_item_t *item, xj_pool_t *pool);
int xj_parsefile_2(const char *path, char *error_buffer, size_t error_buffer_size, size_t *error_offset, size_t *error_column, size_t *error_lineno, int flags, xj_item_t *item, xj_pool_t *pool);
void xj_dump(xj_item_t item, FILE *f);
void xj_free(xj_pool_t *pool);
size_t xj_length(xj_item_t item);
int xj_foreach(xj_item_t item, int *i, char **key, xj_item_t *child_item);
const char *xj_tocstring(const xj_item_t *item);