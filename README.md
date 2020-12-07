
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
	xj_pool_t pool;
	xj_item_t item;

	if(!xj_parse(source, length, &item, &pool)) {

		fprintf(stderr, "Failed!\n");
		return -1;
	}

	xj_item_t number_item = xj_query(item, "some_data.numbers[%d]", 1);
    
    if(number_item.type & xj_INT) {
        
        printf("The number is %lld!", number_item.value.as_int);
        
    } else {
    
    }
    
	printf("Done!\n");


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