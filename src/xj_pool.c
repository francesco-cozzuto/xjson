
#include "xj.h"

#if xj_VALGRIND_DEBUG
#include <valgrind/memcheck.h>
#endif

void xj_pool_setup(xj_pool_t *pool)
{
	pool->tail = &pool->head;
	pool->head.next = NULL;
	pool->head.size = xj_POOL_CHUNK_SIZE_IN_BYTES;
	pool->tail_used = 0;

#if xj_VALGRIND_DEBUG
	pool->head.size -= xj_VALGRIND_PADDING;
	VALGRIND_CREATE_MEMPOOL(pool, xj_VALGRIND_PADDING, 0);
	VALGRIND_MAKE_MEM_NOACCESS(pool->head.body, xj_POOL_CHUNK_SIZE_IN_BYTES - xj_VALGRIND_PADDING);
#endif
}

void xj_free(xj_pool_t *pool)
{
	xj_pool_release(pool);
}

void xj_pool_release(xj_pool_t *pool)
{
	xj_pool_chunk_t *chunk = pool->head.next;
	
#if xj_VALGRIND_DEBUG
		VALGRIND_MEMPOOL_FREE(pool, &pool->head);
#endif

	while(chunk) {

		xj_pool_chunk_t *next_chunk = chunk->next;

		free(chunk);

		chunk = next_chunk;
	}

#if xj_VALGRIND_DEBUG
	VALGRIND_DESTROY_MEMPOOL(pool);
#endif
}

void *xj_pool_request(xj_pool_t *pool, size_t size, int aligned)
{
	// If it's requested, align the chunk pointer to 8 bytes

	if(aligned && pool->tail_used & 7)
		pool->tail_used = (pool->tail_used & ~7) + 8;


	// Check that there is enough space and, if not, grow the pool

	if(pool->tail_used + size > pool->tail->size) {


		// Append a new chunk

		size_t chunk_size = (size > xj_POOL_CHUNK_SIZE_IN_BYTES) ? size : xj_POOL_CHUNK_SIZE_IN_BYTES;

#if xj_VALGRIND_DEBUG
		chunk_size += xj_VALGRIND_PADDING;
#endif

		xj_pool_chunk_t *chunk = malloc(sizeof(xj_pool_chunk_t) + chunk_size);

		if(chunk == NULL)
			return NULL;

#if xj_VALGRIND_DEBUG
		VALGRIND_MAKE_MEM_NOACCESS(chunk->body, chunk_size);
#endif
		chunk->size = chunk_size;
		chunk->next = NULL;
		pool->tail->next = chunk;

		// Set the new chunk as the current one

		pool->tail = pool->tail->next;
		pool->tail_used = 0;
	}

	void *addr = pool->tail->body + pool->tail_used;

	pool->tail_used += size;

#if xj_VALGRIND_DEBUG
	pool->tail_used += xj_VALGRIND_PADDING;
#endif

#if xj_VALGRIND_DEBUG
VALGRIND_MEMPOOL_ALLOC(pool, addr, size);
#endif

	return addr;
}