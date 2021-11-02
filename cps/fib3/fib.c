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

// Generic/polymorphic frame. Just contains
// the function pointer, that we will explicitly
// embed in the concrete frames.
struct fib_frame
{
    int (*fn)(closure, int);
};

// Function specific frames
static inline int ret(closure cl, int n);
struct ret_frame
{
    int (*fn)(closure, int);
};

static inline int fib1(closure cl, int f1);
struct fib1_frame
{
    int (*fn)(closure, int);
    int n;
    closure k;
};

static inline int fib2(closure cl, int f2);
struct fib2_frame
{
    int (*fn)(closure, int);
    int f1;
    closure k;
};

// A macro now, to make it polymorphic and cast it.
#define get_closure_frame(TYPE, CL) \
    ((TYPE *)vs_get_stack_frame(&closure_stack, CL))

static inline int call_closure(closure cl, int arg)
{
    return get_closure_frame(struct fib_frame, cl)->fn(cl, arg);
}

// Function specific closure constructors
static inline closure new_ret_closure(void)
{
    closure cl = vs_stack_alloc_frame_type(&closure_stack, struct ret_frame);
    get_closure_frame(struct ret_frame, cl)->fn = ret;
    return cl;
}

static inline closure new_fib2_closure(int f1, closure k)
{
    closure cl = vs_stack_alloc_frame_type(&closure_stack, struct fib2_frame);
    *get_closure_frame(struct fib2_frame, cl) =
        (struct fib2_frame){.fn = fib2, .f1 = f1, .k = k};
    return cl;
}

static inline closure new_fib1_closure(int n, closure k)
{
    closure cl = vs_stack_alloc_frame_type(&closure_stack, struct fib1_frame);
    *get_closure_frame(struct fib1_frame, cl) =
        (struct fib1_frame){.fn = fib1, .n = n, .k = k};
    return cl;
}

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

// copies the frame to a local (function stack) variable so it is safe
// to free the frame. Then frees it. To use the variables in the frame,
// access them with _.<var-name>.
#define pop_frame(TYPE, CL)                \
    TYPE _ = *get_closure_frame(TYPE, CL); \
    free_closure(CL);

static inline int fib2(closure cl, int f2)
{
    pop_frame(struct fib2_frame, cl);
    return call_closure(_.k, _.f1 + f2);
}

static inline int fib_cps(int n, closure k);
static inline int fib1(closure cl, int f1)
{
    pop_frame(struct fib1_frame, cl);
    return fib_cps(_.n - 2, new_fib2_closure(f1, _.k));
}

static inline int fib_cps(int n, closure k)
{
    if (n <= 1)
    {
        return call_closure(k, n);
    }
    else
    {
        return fib_cps(n - 1, new_fib1_closure(n, k));
    }
}

static inline int fib(int n)
{
    return fib_cps(n, new_ret_closure());
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