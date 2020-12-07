
#include "xj.h"

void xj_stack_setup(xj_stack_t *stack)
{
	stack->head.prev = NULL;
	stack->head.next = NULL;

	stack->tail = &stack->head;
	stack->tail_used = 0;
	stack->size = 0;
}

void xj_stack_free(xj_stack_t *stack)
{
	xj_stack_chunk_t *chunk = stack->head.next;

	while(chunk) {

		xj_stack_chunk_t *next_chunk = chunk->next;

		free(chunk);

		chunk = next_chunk;
	}
}

int xj_stack_push(xj_stack_t *stack, xj_type_t type, xj_generic_t value)
{
	if(stack->tail_used == xj_STACK_CHUNK_ITEM_COUNT) {

		if(stack->tail->next) {

			stack->tail = stack->tail->next;
			stack->tail_used = 0;

		} else {

			// Add a new chunk

			xj_stack_chunk_t *chunk = malloc(sizeof(xj_stack_chunk_t));

			if(chunk == NULL)
				return 0;

			chunk->prev = stack->tail;
			chunk->next = NULL;
			stack->tail->next = chunk;
			stack->tail = chunk;
			stack->tail_used = 0;
		}
	}

	stack->tail->types[stack->tail_used] = type;
	stack->tail->values[stack->tail_used] = value;

	stack->tail_used++;
	stack->size++;

	return 1;
}

int xj_stack_pop(xj_stack_t *stack, xj_type_t *type, xj_generic_t *value)
{
	if(stack->size == 0)
		return 0;

	if(stack->tail_used == 0) {

		stack->tail = stack->tail->prev;
		stack->tail_used = xj_STACK_CHUNK_ITEM_COUNT;
	}

	stack->tail_used--;
	stack->size--;

	if(type) *type = stack->tail->types[stack->tail_used];
	if(value) *value = stack->tail->values[stack->tail_used];

	return 1;
}