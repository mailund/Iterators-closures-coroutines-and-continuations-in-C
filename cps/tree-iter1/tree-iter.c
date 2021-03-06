#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "stacks.h"
#include "tree.h"

// MARK: Recursive solution
static int tree_sum(struct binary_tree *tree)
{
    if (!tree)
    {
        return 0;
    }
    else
    {
        return tree->val + tree_sum(tree->left) + tree_sum(tree->right);
    }
}

// MARK: CPS solution

// First, various auxilary structs, vars and functions
typedef stack_frame closure;

#define get_closure_frame(TYPE, CL) \
    ((TYPE *)pool_get_stack_frame(CL))

#define pop_frame(TYPE, CL)                \
    TYPE _ = *get_closure_frame(TYPE, CL); \
    free_closure(CL);

// generic caller...
#define call_closure(TYPE, CL, ...) \
    get_closure_frame(TYPE, CL)->fn(CL, __VA_ARGS__)

// Same as the closure creator from the fib example with
// statement expressions, but without using the compiler
// extension. We will use it to build frame-specific
// constructors.
#define return_new_closure(STACK, TYPE, FN, ...)                  \
    closure cl = pool_stack_alloc_frame_type(STACK, TYPE);        \
    *get_closure_frame(TYPE, cl) = (TYPE){.fn = FN, __VA_ARGS__}; \
    return cl;

static inline void free_closure(closure cl)
{
    pool_stack_free_frame(cl);
}

// MARK: Application code...
static int cps_sum(struct binary_tree *tree, closure k);

typedef int (*tree_fun)(closure, int);
struct tree_frame
{
    tree_fun fn;
};

static inline int ret(closure cl, int res);
struct ret_frame
{
    tree_fun fn;
};
static inline closure new_ret_closure(stack_id stack)
{
    return_new_closure(stack, struct ret_frame, ret);
}

static inline int visit(closure cl, int left_sum);
struct visit_frame
{
    tree_fun fn;
    struct binary_tree *tree;
    closure k;
};
static inline closure new_visit_closure(stack_id stack,
                                        struct binary_tree *tree,
                                        closure k)
{
    return_new_closure(stack, struct visit_frame, visit,
                       .tree = tree, .k = k);
}

static inline int add(closure cl, int right_sum);
struct add_frame
{
    tree_fun fn;
    int left_sum;
    closure k;
};
static inline closure new_add_closure(stack_id stack,
                                      int left_sum,
                                      closure k)
{
    return_new_closure(stack, struct add_frame, add,
                       .left_sum = left_sum, .k = k);
}

// And then the actual application
static inline int ret(closure cl, int n)
{
    free_closure(cl);
    return n;
}

static inline int visit(closure cl, int left_sum)
{
    pop_frame(struct visit_frame, cl);
    return cps_sum(_.tree->right,
                   new_add_closure(_.k.stack, left_sum + _.tree->val, _.k));
}

static inline int add(closure cl, int right_sum)
{
    pop_frame(struct add_frame, cl);
    return call_closure(struct tree_frame, _.k, _.left_sum + right_sum);
}

static inline int cps_sum(struct binary_tree *tree, closure k)
{
    if (!tree)
    {
        return call_closure(struct tree_frame, k, 0);
    }
    else
    {
        return cps_sum(tree->left,
                       new_visit_closure(k.stack, tree, k));
    }
}

static inline int sum(struct binary_tree *tree)
{
    stack_id stack = pool_alloc_stack();
    int res = cps_sum(tree, new_ret_closure(stack));
    pool_dealloc_stack(stack);
    return res;
}

// MARK: Iterator
struct tree_iter
{
    bool has_more;
    int current_val;
    stack_id stack;
    closure next;
};

static void traverse(struct tree_iter *itr,
                     struct binary_tree *tree,
                     closure k);

typedef void (*tree_iter_fun)(closure, struct tree_iter *);
struct tree_iter_frame
{
    tree_iter_fun fn;
};

static void done(closure cl, struct tree_iter *itr);
struct done_frame
{
    tree_iter_fun fn;
};
static inline closure new_done_closure(stack_id stack)
{
    return_new_closure(stack, struct done_frame, done);
}

static void yield(closure cl, struct tree_iter *itr);
struct yield_frame
{
    tree_iter_fun fn;
    struct binary_tree *tree;
    closure k;
};
static inline closure new_yield_closure(stack_id stack,
                                        struct binary_tree *tree,
                                        closure k)
{
    return_new_closure(stack, struct yield_frame, yield,
                       .tree = tree, .k = k);
}

static void resume(closure cl, struct tree_iter *itr);
struct resume_frame
{
    tree_iter_fun fn;
    struct binary_tree *tree;
    closure k;
};
static inline closure new_resume_closure(stack_id stack,
                                         struct binary_tree *tree,
                                         closure k)
{
    return_new_closure(stack, struct resume_frame, resume,
                       .tree = tree, .k = k);
}

static void done(closure cl, struct tree_iter *itr)
{
    free_closure(cl);
    itr->has_more = false;
}

static void yield(closure cl, struct tree_iter *itr)
{
    pop_frame(struct yield_frame, cl);
    itr->has_more = true;
    itr->current_val = _.tree->val;
    itr->next = new_resume_closure(_.k.stack, _.tree->right, _.k);
}

static void resume(closure cl, struct tree_iter *itr)
{
    pop_frame(struct resume_frame, cl);
    traverse(itr, _.tree, _.k);
}

static void traverse(struct tree_iter *itr,
                     struct binary_tree *tree,
                     closure k)
{
    if (!tree)
    {
        call_closure(struct tree_iter_frame, k, itr);
    }
    else
    {
        traverse(itr, tree->left,
                 new_yield_closure(k.stack, tree, k));
    }
}

// Implementing an iterator from the continuations
static void init_iter(struct tree_iter *itr, struct binary_tree *tree)
{
    itr->stack = pool_alloc_stack();
    closure k = new_done_closure(itr->stack);
    traverse(itr, tree, k); // traverse to first hit
}

static void next(struct tree_iter *itr)
{
    call_closure(struct tree_iter_frame, itr->next, itr);
}

static void free_iter(struct tree_iter *itr)
{
    pool_dealloc_stack(itr->stack);
}

// MARK: Go!
int main(void)
{
    struct binary_tree *tree =
        new_node(1, new_node(2, NULL, NULL),
                 new_node(3, new_node(4, NULL, NULL),
                          NULL));

    printf("rec: %d, cps: %d\n", tree_sum(tree), sum(tree));
    assert(tree_sum(tree) == sum(tree));

    int s = 0;
    struct tree_iter itr;
    init_iter(&itr, tree);
    for (; itr.has_more; next(&itr))
    {
        printf("%d ", itr.current_val);
        s += itr.current_val;
    }
    putchar('\n');
    free_iter(&itr);

    assert(s == sum(tree));

    return 0;
}
