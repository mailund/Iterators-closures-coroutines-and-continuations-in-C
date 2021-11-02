#include <stdlib.h>

#include "stacks.h"

#define INIT_STACK_SIZE 512 /* completely arbitrary */

void vs_init_stack(struct vs_stack *stack)
{
    stack->sp = 0;
    stack->data = malloc(INIT_STACK_SIZE);
    stack->size = INIT_STACK_SIZE;
}

void vs_resize_stack(struct vs_stack *stack, size_t new_size)
{
    // Don't make the stack too small...
    if (new_size < INIT_STACK_SIZE)
        new_size = INIT_STACK_SIZE;

    stack->data = realloc(stack->data, new_size);
    stack->size = new_size;
}

void vs_free_stack(struct vs_stack *stack)
{
    free(stack->data);
    stack->data = 0; // just in case.
}
