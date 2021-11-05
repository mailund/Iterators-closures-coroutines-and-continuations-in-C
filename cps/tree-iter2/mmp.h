
#ifndef MMP_H
#define MMP_H

#define mmp_first(a, ...) a
#define mmp_second(a, b, ...) b

#define mmp_cat(x, y) x##y

#define mmp_head mmp_first
#define mmp_tail(a, ...) __VA_ARGS__

#define mmp_test(p) mmp_second(p, 0)
#define mmp_is_true() -, 1

#define mmp_not(b) mmp_test(mmp_cat(mmp_not_, b))
#define mmp_not_0 mmp_is_true()
#define mmp_truthiness(b) mmp_not(mmp_not(b))

#define mmp_if_else(b) mmp_if_else_(mmp_truthiness(b))
#define mmp_if_else_(b) mmp_cat(mmp_if_, b)
#define mmp_if_0(...) mmp_else_0
#define mmp_if_1(...) __VA_ARGS__ mmp_else_1
#define mmp_else_0(...) __VA_ARGS__
#define mmp_else_1(...)

#define mmp_is_nil(...) mmp_test(mmp_first(mmp_is_true __VA_ARGS__)())

#define mmp_expand_1(...) __VA_ARGS__
#define mmp_expand_2(...) mmp_expand_1(mmp_expand_1(__VA_ARGS__))
#define mmp_expand_4(...) mmp_expand_2(mmp_expand_2(__VA_ARGS__))
#define mmp_expand_8(...) mmp_expand_4(mmp_expand_4(__VA_ARGS__))
#define mmp_expand_16(...) mmp_expand_8(mmp_expand_8(__VA_ARGS__))
#define mmp_expand_32(...) mmp_expand_16(mmp_expand_16(__VA_ARGS__))
#define mmp_expand_64(...) mmp_expand_32(mmp_expand_32(__VA_ARGS__))
#define mmp_eval mmp_expand_64

#define mmp_empty()
#define mmp_delay(f) f mmp_empty()
#define mmp_delay2(f) f mmp_empty mmp_empty()()

// clang-format off
#define mmp_map(f, ...) \
    mmp_eval(mmp_map_(f, __VA_ARGS__))
#define mmp_map__() mmp_map_
#define mmp_map_(f, ...) \
    mmp_if_else(mmp_is_nil(__VA_ARGS__))\
    ()\
    (f(mmp_head(__VA_ARGS__)) \
    mmp_delay2(mmp_map__)()(f, mmp_tail(__VA_ARGS__)))

// map over the elements in a list pairwise
#define mmp_map2(f, ...) \
    mmp_eval(mmp_map2_(f, __VA_ARGS__))
#define mmp_map2__() mmp_map2_
#define mmp_map2_(f, ...) \
    mmp_if_else(mmp_is_nil(__VA_ARGS__))\
    ()\
    (f(mmp_head(__VA_ARGS__), mmp_second(__VA_ARGS__)) \
    mmp_delay2(mmp_map2__)()(f, mmp_take2(__VA_ARGS__)))
#define mmp_take2(a, b, ...) __VA_ARGS__
// clang-format on

#define mmp_prepend_comma(x) , x
#define mmp_prepend_comma_if_not_nil(...) \
    mmp_if_else(mmp_is_nil(__VA_ARGS__))()(, ) __VA_ARGS__

#endif // MMP_H
