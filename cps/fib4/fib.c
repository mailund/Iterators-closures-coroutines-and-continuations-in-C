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
typedef stack_frame closure; // a closure is a frame from the pool

typedef int (*fib_fun)(closure, int);
struct fib_frame
{
    fib_fun fn;
    int n, f1;
    closure k;
};

static inline struct fib_frame *get_closure_frame(closure cl)
{
    return pool_get_stack_frame(cl);
}

// a little simpler version since we only have one
// type of frames now.
#define pop_frame(CL)                            \
    struct fib_frame _ = *get_closure_frame(CL); \
    free_closure(CL);

static inline int call_closure(closure cl, int arg)
{
    return get_closure_frame(cl)->fn(cl, arg);
}

static inline closure new_closure_from_struct(stack_id stack,
                                              fib_fun fn,
                                              struct fib_frame data)
{
    closure cl = pool_stack_alloc_frame_type(stack, struct fib_frame);
    struct fib_frame *frame = get_closure_frame(cl);
    *frame = data;
    frame->fn = fn;
    return cl;
}

#define new_closure(STACK, FN, ...) \
    new_closure_from_struct(STACK, FN, (struct fib_frame){__VA_ARGS__})

static inline void free_closure(closure cl)
{
    pool_stack_free_frame(cl);
}

// And then the actual application
static inline int ret(closure cl, int n)
{
    free_closure(cl);
    return n;
}

static inline int fib2(closure cl, int f2)
{
    pop_frame(cl);
    return call_closure(_.k, _.f1 + f2);
}

static inline int fib_cps(int n, closure k);

static inline int fib1(closure cl, int f1)
{
    pop_frame(cl);
    return fib_cps(_.n - 2,
                   new_closure(cl.stack, fib2, .f1 = f1, .k = _.k));
}

static inline int fib_cps(int n, closure k)
{
    if (n <= 1)
    {
        return call_closure(k, n);
    }
    else
    {
        return fib_cps(n - 1, new_closure(k.stack, fib1, .n = n, .k = k));
    }
}

static inline int fib(int n)
{
    stack_id stack = pool_alloc_stack();
    int res = fib_cps(n, new_closure(stack, ret, ));
    pool_dealloc_stack(stack);
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