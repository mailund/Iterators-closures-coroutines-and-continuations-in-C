
#ifndef CLOSURES_H
#define CLOSURES_H

#include "mmp.h"
#include "stacks.h"

// First, various auxilary structs, vars and functions
typedef stack_frame closure;
// special closure indicating no closure
const closure nil_closure = {.stack = 0xFFFF, .sp = 0xFFFFFFFFFFFF};
// The compiler should make this a single 64-bit integer cmp.
static inline bool is_nil_closure(closure cl)
{
    return cl.stack == nil_closure.stack && cl.sp == nil_closure.sp;
}

typedef void (*generic_fun)(closure, ...);
struct generic_frame
{
    // Function at the top...
    generic_fun fn;
    // then data goes here...
};

#define get_closure_frame(TYPE, CL) \
    ((TYPE *)pool_get_stack_frame(CL))

#define pop_frame(TYPE, CL)                \
    TYPE _ = *get_closure_frame(TYPE, CL); \
    free_closure(CL);

// Using function type for cast intead of frame
#define call_closure(FUNTYPE, CL, ...) \
    ((FUNTYPE)get_closure_frame(struct generic_frame, CL)->fn)(CL mmp_prepend_comma_if_not_nil(__VA_ARGS__))

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
