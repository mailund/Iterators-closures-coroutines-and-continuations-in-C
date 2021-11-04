
#ifndef CLOSURES_H
#define CLOSURES_H

#include "mmp.h"
#include "stacks.h"

// First, various auxilary structs, vars and functions
typedef stack_frame closure;

#define get_closure_frame(TYPE, CL) \
    ((TYPE *)pool_get_stack_frame(CL))

#define pop_frame(TYPE, CL)                \
    TYPE _ = *get_closure_frame(TYPE, CL); \
    free_closure(CL);

// generic caller...
// FIXME: handle empty ...
#define call_closure(TYPE, CL, ...) \
    get_closure_frame(TYPE, CL)->fn(CL, __VA_ARGS__)

static inline void free_closure(closure cl)
{
    pool_stack_free_frame(cl);
}

// Same as the closure creator from the fib example with
// statement expressions, but without using the compiler
// extension. We will use it to build frame-specific
// constructors.
#define return_new_closure(STACK, TYPE, FN, ...)                  \
    closure cl = pool_stack_alloc_frame_type(STACK, TYPE);        \
    *get_closure_frame(TYPE, cl) = (TYPE){.fn = FN, __VA_ARGS__}; \
    return cl;

#define args(...) __VA_ARGS__
#define members(...) __VA_ARGS__
#define field(TYPE, NAME) TYPE, NAME
#define prototype(QUAL, RET, ...) QUAL, RET, __VA_ARGS__

#define declare_member(TYPE, NAME) TYPE NAME;
#define member_arg(TYPE, NAME) , TYPE NAME
#define assign_member_arg(_, NAME) , .NAME = NAME

// clang-format off
#define declare_new_closure(NAME, PROTO, FIELDS)                                 \
    mmp_first(PROTO)                                                             \
    mmp_second(PROTO) NAME(closure                                               \
                           mmp_map(mmp_prepend_comma, mmp_take2(PROTO)));        \
    struct NAME##_frame                                                          \
    {                                                                            \
        mmp_second(PROTO) (*fn)(closure                                          \
                                mmp_map(mmp_prepend_comma, mmp_take2(PROTO)));   \
        mmp_map2(declare_member, FIELDS)                                         \
    };                                                                           \
    static inline                                                                \
    closure new_##NAME##_closure(stack_id stack mmp_map2(member_arg, FIELDS))    \
    {                                                                            \
        return_new_closure(stack, struct NAME##_frame, NAME                      \
                           mmp_map2(assign_member_arg, FIELDS));                 \
    }
// clang-format on


#endif // CLOSURES_H