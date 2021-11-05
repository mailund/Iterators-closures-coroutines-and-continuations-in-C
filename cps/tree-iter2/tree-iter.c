#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "closures.h"
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

// MARK: Iterator
// Implementing an iterator from the continuations, where
// the continuations do not need to know the iterator implementation
// and the iterator doesn't need to know about the concrete continuations.
#define iterator(TYPE)    \
    struct                \
    {                     \
        TYPE current_val; \
        stack_id stack;   \
        closure next;     \
    }

#define new_iterator()              \
    {                               \
        .stack = pool_alloc_stack() \
    }

// Prototype for resume functions. The next(it) operator
// will only call the next closure with itself; any additional
// data should be stored in the closure when we create it.
typedef closure (*itr_next_fun)(closure);

// Termination closure. It is application independent, but should
// handle whatever function type the application uses for
// continuations, so we use variable length arguments, ..., and
// then just ignore the arguments.
declare_new_closure(
    done,
    prototype(static, closure, ...),
    members());
static closure done(closure cl, ...)
{
    free_closure(cl);
    return nil_closure;
}

#define iterate_with(it, cps, ...)                                            \
    (it.next = cps(&it.current_val, __VA_ARGS__, new_done_closure(it.stack)), \
     it.current_val)

#define next(it) \
    (it.next = call_closure(itr_next_fun, it.next), it.current_val)

#define has_more(it) \
    !is_nil_closure(it.next)

#define free_iterator(itr) \
    pool_dealloc_stack(itr.stack)

// MARK: application iteration code
static closure traverse(int *val,
                        struct binary_tree *tree,
                        closure k);

typedef closure (*int_iter_fun)(closure, int *);

declare_new_closure(
    yield,
    prototype(static, closure, int *val),
    members(field(struct binary_tree *, tree),
            field(closure, k)));
declare_new_closure(
    resume,
    prototype(static, closure, ),
    members(field(int *, val),
            field(struct binary_tree *, tree),
            field(closure, k)));

static closure yield(closure cl, int *val)
{
    pop_frame(struct yield_frame, cl);
    *val = _.tree->val;
    return new_resume_closure(_.k.stack, val, _.tree->right, _.k);
}

static closure resume(closure cl)
{
    pop_frame(struct resume_frame, cl);
    return traverse(_.val, _.tree, _.k);
}

static closure traverse(int *val,
                        struct binary_tree *tree,
                        closure k)
{
    if (!tree)
    {
        return call_closure(int_iter_fun, k, val);
    }
    else
    {
        return traverse(val, tree->left,
                        new_yield_closure(k.stack, tree, k));
    }
}

// MARK: list of doubles just to try iterating over a different
// type. CPS without multiple recursions is silly, but it is just
// a test...
struct list
{
    double d;
    struct list *next;
};

static struct list *new_link(double d, struct list *next)
{
    struct list *link = malloc(sizeof *link);
    *link = (struct list){.d = d, .next = next};
    return link;
}

typedef closure (*double_iter_fun)(closure, double *);
static closure traverse_list_vals(double *val,
                                  struct list *list,
                                  closure k);

declare_new_closure(
    resume_double,
    prototype(static, closure, ),
    members(field(double *, val),
            field(struct list *, list),
            field(closure, k)));

static closure resume_double(closure cl)
{
    pop_frame(struct resume_double_frame, cl);
    return traverse_list_vals(_.val, _.list, _.k);
}

static closure traverse_list_vals(double *val,
                                  struct list *list,
                                  closure k)
{
    if (!list)
    {
        return call_closure(double_iter_fun, k, val);
    }
    else
    {
        *val = list->d;
        return new_resume_double_closure(k.stack, val, list->next, k);
    }
}

typedef closure (*link_iter_fun)(closure, struct list **);
static closure traverse_list_links(struct list **val,
                                   struct list *list,
                                   closure k);

declare_new_closure(
    resume_link,
    prototype(static, closure),
    members(field(struct list **, val),
            field(struct list *, list),
            field(closure, k)));

static closure resume_link(closure cl)
{
    pop_frame(struct resume_link_frame, cl);
    return traverse_list_links(_.val, _.list, _.k);
}

static closure traverse_list_links(struct list **val,
                                   struct list *list,
                                   closure k)
{
    if (!list)
    {
        return call_closure(link_iter_fun, k, val);
    }
    else
    {
        *val = list;
        return new_resume_link_closure(k.stack, val, list->next, k);
    }
}

// MARK: Go!
int main(void)
{
    struct binary_tree *tree =
        new_node(1, new_node(2, NULL, NULL),
                 new_node(3, new_node(4, NULL, NULL),
                          NULL));

    int s = 0;
    iterator(int) it = new_iterator();
    for (int i = iterate_with(it, traverse, tree); has_more(it); i = next(it))
    {
        printf("%d ", i);
        s += i;
    }
    printf("\n");
    free_iterator(it);

    assert(s == tree_sum(tree));
    free_binary_tree(tree);

    struct list *list = new_link(1, new_link(2, new_link(3, NULL)));
    iterator(double) itr_d = new_iterator();
    for (double d = iterate_with(itr_d, traverse_list_vals, list);
         has_more(itr_d); d = next(itr_d))
    {
        printf("%f ", d);
    }
    printf("\n");
    free_iterator(itr_d);

    iterator(struct list *) itr_link = new_iterator();
    for (struct list *link = iterate_with(itr_link, traverse_list_links, list);
         has_more(itr_link); link = next(itr_link))
    {
        printf("%p ", link);
        free(link);
    }
    printf("\n");
    free_iterator(itr_d);

    return 0;
}
