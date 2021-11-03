#include <assert.h>
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

// FIXME: don't use this compiler extension...
// clang-format off
#define new_closure(STACK, TYPE, ...)                          \
    ({                                                         \
        closure cl_ = pool_stack_alloc_frame_type(STACK, TYPE); \
        *get_closure_frame(TYPE, cl_) = (TYPE){__VA_ARGS__};    \
        cl_;                                                    \
     })
// clang-format on

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
struct ret_frame {
    tree_fun fn;
};

static inline int visit(closure cl, int left_sum);
struct visit_frame {
    tree_fun fn;
    struct binary_tree *tree;
    closure k;
};

static inline int add(closure cl, int right_sum);
struct add_frame {
    tree_fun fn;
    int left_sum;
    closure k;
};

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
                   new_closure(_.k.stack, struct add_frame,
                               .fn = add,
                               .left_sum = left_sum + _.tree->val,
                               .k = _.k));
}

static inline int add(closure cl, int right_sum)
{
    pop_frame(struct add_frame, cl);
    return call_closure(struct tree_frame, _.k, _.left_sum + right_sum);
}

static inline int cps_sum(struct binary_tree *tree, closure k)
{
    if (!tree) {
        return call_closure(struct tree_frame, k, 0);
    } else {
        return cps_sum(tree->left,
                       new_closure(k.stack, struct visit_frame,
                                   .fn = visit, .tree = tree, .k = k));
    }
}

static inline int sum(struct binary_tree *tree)
{
    stack_id stack = pool_alloc_stack();
    int res = cps_sum(tree, new_closure(stack, struct ret_frame, ret, ));
    pool_dealloc_stack(stack);
    return res;
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
    
    return 0;
}
