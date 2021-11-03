#ifndef STACKS_H
#define STACKS_H

#include <stdalign.h>
#include <stdlib.h>

// MARK: Variable frame size stack
struct vs_stack
{
    size_t size, sp;
    void *data;
};

void vs_init_stack(struct vs_stack *stack);
void vs_resize_stack(struct vs_stack *stack, size_t new_size);
void vs_free_stack(struct vs_stack *stack);

// move an address up to the nearest that matches the alignment
// constraints.
static inline size_t vs_align_address(size_t addr, size_t align)
{
    // align is a power of two, so   00...000100.
    // thus m becomes                00...000011.
    // If addr is zero on the lower bits and we
    // add m, we leave the upper bits alone.
    // However, if addr has set lower bits:
    // add m to addr: if addr = 10...011001
    //                     +m   00...000011
    //               addr + m = 10...100101
    // will add 1 to the bits above align.
    // masking out the lower bits and we have
    // rounded up.
    size_t m = align - 1;
    return (addr + m) & ~m;
}

static inline size_t vs_stack_alloc_frame_aligned(struct vs_stack *stack,
                                                  size_t frame_size,
                                                  size_t frame_align)
{
    stack->sp = vs_align_address(stack->sp, frame_align);
    if (stack->sp + frame_size > stack->size)
    {
        vs_resize_stack(stack, 2 * (stack->sp + frame_size));
    }

    size_t new_frame = stack->sp;
    stack->sp += frame_size;

    return new_frame;
}

#define vs_stack_alloc_frame_type(STACK, FRAME_TYPE) \
    vs_stack_alloc_frame_aligned(STACK, sizeof(FRAME_TYPE), alignof(FRAME_TYPE))

static inline void vs_stack_free_frame(struct vs_stack *stack,
                                       size_t fp)
{
    stack->sp = fp;
    if (stack->sp < stack->size / 4)
    {
        vs_resize_stack(stack, stack->size / 2);
    }
}

static inline void *vs_get_stack_frame(struct vs_stack *stack, size_t offset)
{
    return (char *)stack->data + offset;
}

// MARK: Pool of stacks

typedef uint16_t stack_id;
typedef struct
{
    // If this doesn't get packed into a 64 bit integer,
    // use bit shifts and masks to get the same effect.
    stack_id stack;
    uint64_t sp : 48;
} stack_frame;

extern struct stack_pool
{
    union stack_free_list
    {
        struct vs_stack stack;
        union stack_free_list *free; // free list
    } * stacks;
    union stack_free_list *free; // free list
    size_t no_stacks;
} stack_pool;

stack_id pool_alloc_stack(void);
void pool_dealloc_stack(stack_id stack);

static inline stack_frame pool_stack_alloc_frame_aligned(stack_id stack,
                                                         size_t frame_size,
                                                         size_t frame_align)
{
    size_t sp = vs_stack_alloc_frame_aligned(&stack_pool.stacks[stack].stack,
                                             frame_size, frame_align);
    return (stack_frame){.stack = stack, .sp = sp};
}

#define pool_stack_alloc_frame_type(STACK_ID, FRAME_TYPE) \
    pool_stack_alloc_frame_aligned(STACK_ID, sizeof(FRAME_TYPE), alignof(FRAME_TYPE))

static inline void pool_stack_free_frame(stack_frame frame)
{
    vs_stack_free_frame(&stack_pool.stacks[frame.stack].stack, frame.sp);
}

static inline void *pool_get_stack_frame(stack_frame frame)
{
    return vs_get_stack_frame(&stack_pool.stacks[frame.stack].stack, frame.sp);
}

#endif // STACKS_H
