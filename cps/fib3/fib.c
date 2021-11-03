#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stacks.h"

// MARK: Recursive solution
static int fib_rec(int n)
{
    if (n <= 1)
    {
        return n;
    }
    else
    {
        return fib_rec(n - 1) + fib_rec(n - 2);
    }
}

// MARK: CPS solution

// First, various auxilary structs, vars and functions
typedef size_t closure; // a closure is an offset into a stack.

typedef int (*fib_fun)(struct vs_stack *, closure, int);
struct fib_frame
{
    fib_fun fn;
    int n, f1;
    closure k;
};

static inline struct fib_frame *get_closure_frame(struct vs_stack *stack, closure cl)
{
    return vs_get_stack_frame(stack, cl);
}

static inline int call_closure(struct vs_stack *stack, closure cl, int arg)
{
    return get_closure_frame(stack, cl)->fn(stack, cl, arg);
}

static inline closure new_closure_from_struct(struct vs_stack *stack,
                                              fib_fun fn,
                                              struct fib_frame data)
{
    closure cl = vs_stack_alloc_frame_type(stack, struct fib_frame);
    struct fib_frame *frame = get_closure_frame(stack, cl);
    *frame = data;
    frame->fn = fn;
    return cl;
}

#define new_closure(STACK, FN, ...) \
    new_closure_from_struct(STACK, FN, (struct fib_frame){__VA_ARGS__})

static inline void free_closure(struct vs_stack *stack, closure cl)
{
    vs_stack_free_frame(stack, cl);
}

// a little simpler version since we only have one
// type of frames now.
#define pop_frame(STACK, CL)                            \
    struct fib_frame _ = *get_closure_frame(STACK, CL); \
    free_closure(STACK, CL);

// And then the actual application
static inline int ret(struct vs_stack *stack, closure cl, int n)
{
    free_closure(stack, cl);
    return n;
}

static inline int fib2(struct vs_stack *stack, closure cl, int f2)
{
    pop_frame(stack, cl);
    return call_closure(stack, _.k, _.f1 + f2);
}

static inline int fib_cps(struct vs_stack *stack, int n, closure k);

static inline int fib1(struct vs_stack *stack, closure cl, int f1)
{
    pop_frame(stack, cl);
    return fib_cps(stack, _.n - 2,
                   new_closure(stack, fib2, .f1 = f1, .k = _.k));
}

static inline int fib_cps(struct vs_stack *stack, int n, closure k)
{
    if (n <= 1)
    {
        return call_closure(stack, k, n);
    }
    else
    {
        return fib_cps(stack, n - 1,
                       new_closure(stack, fib1, .n = n, .k = k));
    }
}

static inline int fib(int n)
{
    struct vs_stack stack;
    vs_init_stack(&stack);
    int res = fib_cps(&stack, n, new_closure(&stack, ret, ));
    vs_free_stack(&stack);
    return res;
}

// MARK: Go!
int main(void)
{
    for (int n = 0; n < 10; n++)
    {
        printf("fib_rec(%d) == %d, fib(%d) == %d\n", n, fib_rec(n), n, fib(n));
        assert(fib_rec(n) == fib(n));
    }

    return 0;
}