#include <assert.h>
#include <stdio.h>
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

// MARK: pools

// declaring global var for stack pools.
struct stack_pool stack_pool;

#define INIT_POOL_SIZE 4

static void set_free_list(union stack_free_list *frames, size_t n)
{
    for (size_t i = 0; i < n - 1; i++)
    {
        frames->free = frames + 1;
        frames++;
    }
    frames->free = 0;
}

// FIXME: implement these!
stack_id pool_alloc_stack(void)
{
    if (!stack_pool.stacks)
    {
        stack_pool.stacks = malloc(INIT_POOL_SIZE * sizeof *stack_pool.stacks);
        set_free_list(stack_pool.stacks, INIT_POOL_SIZE);
        stack_pool.free = stack_pool.stacks;
    }
    
    if (!stack_pool.free)
    {
        if (stack_pool.no_stacks == 1 << 16)
        {
            perror("exceeded stack pool.");
            abort();
        }
        // Grow pool to twice the size.
        stack_pool.stacks =
            realloc(stack_pool.stacks,
                    2 * stack_pool.no_stacks * sizeof *stack_pool.stacks);
        set_free_list(stack_pool.stacks + stack_pool.no_stacks,
                      stack_pool.no_stacks);
        stack_pool.free = stack_pool.stacks + stack_pool.no_stacks;
        stack_pool.no_stacks *= 2;
    }

    union stack_free_list *next_free = stack_pool.free->free;
    stack_id new_stack = (stack_id)(stack_pool.free - stack_pool.stacks);
    vs_init_stack(&stack_pool.free->stack);
    stack_pool.free = next_free;
    stack_pool.no_stacks++;
    return new_stack;
}

void pool_dealloc_stack(stack_id stack)
{
    union stack_free_list *s = stack_pool.stacks + stack;
    vs_free_stack(&s->stack);
    s->free = stack_pool.free;
    stack_pool.free = s;
    stack_pool.no_stacks--;
}
