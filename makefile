
FLAGS = -Dxj_VALGRIND_DEBUG=1 -g -Wall -Wextra

all: xjson.so test

xjson.so: src/xj.h src/xj_context.c src/xj_parse.c src/xj_pool.c src/xj_query.c src/xj_stack.c src/xj_utils.c
	gcc -shared -fPIC -o xjson.so src/xj_context.c src/xj_parse.c src/xj_pool.c src/xj_query.c src/xj_stack.c src/xj_utils.c $(FLAGS)

test: xjson.so tests/test.c
	gcc -o test xjson.so tests/test.c -I./include/ -Wl,-rpath=./ -g