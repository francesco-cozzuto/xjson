
# xJSON

A nice little C library to parse JSON.

## What it looks like


```c
#include <stdio.h>
#include <xjson.h>

const char source[] = "{ \"some_data\": { \"name\": \"Francesco\", \"numbers\": [22, 33, 44]}}";
size_t length = sizeof(source)-1;

int main()
{

	xj_result_t result = xj_parse(source, length);

	if(result.failed) {

		fprintf(stderr, "Error in %ld:%ld, offset %ld: %s\n", 
			result.column, 
			result.lineno, 
			result.offset, 
			result.message);

		xj_done(result);
		return -1;
	}


	xj_item_t item = xj_query(result.root, "some_data.numbers[%d]", 1);
    
    if(xj_is_int(item))
	    printf("The number is %lld!", item.as_int);


	printf("Bye!\n");

	xj_free(&pool);
	return 0;
}
```
## Table of contents
* License
* Features
* Usage (lots of examples!)
* Tests
* Contribute
* Credits