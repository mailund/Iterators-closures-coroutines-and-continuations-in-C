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
static struct vs_stack closure_stack;
typedef size_t closure; // a closure is an offset into a stack.

struct fib_frame
{
    int (*fn)(closure, int);
    int n, f1;
    closure k;
};

static inline struct fib_frame *get_closure_frame(closure cl)
{
    return vs_get_stack_frame(&closure_stack, cl);
}

static inline int call_closure(closure cl, int arg)
{
    return get_closure_frame(cl)->fn(cl, arg);
}

static inline closure new_closure_from_struct(int (*fn)(closure, int), struct fib_frame data)
{
    closure cl = vs_stack_alloc_frame_type(&closure_stack, struct fib_frame);
    struct fib_frame *frame = get_closure_frame(cl);
    *frame = data;
    frame->fn = fn;
    return cl;
}

#define new_closure(FN, ...) new_closure_from_struct(FN, (struct fib_frame){__VA_ARGS__})

static inline void free_closure(closure cl)
{
    vs_stack_free_frame(&closure_stack, cl);
}

// And then the actual application
static inline int ret(closure cl, int n)
{
    free_closure(cl);
    return n;
}

static inline int fib2(closure cl, int f2)
{
    struct fib_frame *frame = get_closure_frame(cl);
    int f1 = frame->f1;
    closure k = frame->k;

    free_closure(cl);
    return call_closure(k, f1 + f2);
}

static inline int fib_cps(int n, closure k);

static inline int fib1(closure cl, int f1)
{
    struct fib_frame *frame = get_closure_frame(cl);
    int n = frame->n;
    closure k = new_closure(fib2, .f1 = f1, .k = frame->k);

    free_closure(cl);
    return fib_cps(n - 2, k);
}

static inline int fib_cps(int n, closure k)
{
    if (n <= 1)
    {
        return call_closure(k, n);
    }
    else
    {
        return fib_cps(n - 1, new_closure(fib1, .n = n, .k = k));
    }
}

static inline int fib(int n)
{
    return fib_cps(n, new_closure(ret, ));
}

// MARK: Go!
int main(void)
{
    // global stack setup
    vs_init_stack(&closure_stack);

    for (int n = 0; n < 10; n++)
    {
        printf("fib_rec(%d) == %d, fib(%d) == %d\n", n, fib_rec(n), n, fib(n));
        assert(fib_rec(n) == fib(n));
    }

    // global stack cleanup
    vs_free_stack(&closure_stack);

    return 0;
}