#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

#define M_EVAL(...)  M_EVAL1(M_EVAL1(M_EVAL1(M_EVAL1(__VA_ARGS__))))
#define M_EVAL1(...) M_EVAL2(M_EVAL2(M_EVAL2(M_EVAL2(__VA_ARGS__))))
#define M_EVAL2(...) M_EVAL3(M_EVAL3(M_EVAL3(M_EVAL3(__VA_ARGS__))))
#define M_EVAL3(...) __VA_ARGS__

#define _M_CAT(a, b) a ## b
#define M_CAT(a, b) _M_CAT(a, b)

#define _M_STR(...) #__VA_ARGS__
#define M_STR(...) _M_STR(__VA_ARGS__)

#define _M_BOOL(_, n, ...) n
#define M_BOOL(x) M_LATER(_M_BOOL, M_CAT(M_BOOL_, x), 1)
#define M_BOOL_0 _, 0

#define M_IF(c) M_CAT(M_IF_, M_BOOL(c))
#define M_IF_0(t, f) f
#define M_IF_1(t, f) t

#define M_EMPTY(...)
#define M_DEFER(...) __VA_ARGS__ M_EMPTY()
#define M_OBSTRUCT(...) __VA_ARGS__ M_DEFER(M_EMPTY)()
#define M_LATER(x, ...) x(__VA_ARGS__)
#define M_ZERO(...) 0

#define _M_VALEN(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...) _19
#define M_VALEN(...) _M_VALEN(0, ##__VA_ARGS__, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define _M_VASEMI_() _M_VASEMI
#define _M_VASEMI(a, ...) a; M_IF(M_VALEN(__VA_ARGS__))(M_OBSTRUCT(_M_VASEMI_)(), M_EMPTY)(__VA_ARGS__)
#define M_VASEMI(...) M_EVAL(M_IF(M_VALEN(__VA_ARGS__))(_M_VASEMI, M_EMPTY)(__VA_ARGS__))

#define _M_MAP_() _M_MAP
#define _M_MAP(f, i, a, ...) f(a, i, __VA_ARGS__) M_IF(M_VALEN(__VA_ARGS__))(M_OBSTRUCT(_M_MAP_)(), M_EMPTY)(f, i + 1, __VA_ARGS__)
#define M_MAP(f, ...) M_EVAL(M_IF(M_VALEN(__VA_ARGS__))(_M_MAP, M_EMPTY)(f, 0, __VA_ARGS__))

#define _M_MAP2_() _M_MAP2
#define _M_MAP2(f, c, i, a, b, ...) f(a, b, c, i, __VA_ARGS__) M_IF(M_VALEN(__VA_ARGS__))(M_OBSTRUCT(_M_MAP2_)(), M_EMPTY)(f, c, i + 2, __VA_ARGS__)
#define M_MAP2(f, c, ...) M_EVAL(M_IF(M_VALEN(__VA_ARGS__))(_M_MAP2, M_EMPTY)(f, c, 0, __VA_ARGS__))

#define _M_CAT2ARGS_() _M_CAT2ARGS
#define _M_CAT2ARGS(a, b, ...) a b, M_IF(M_VALEN(__VA_ARGS__))(M_OBSTRUCT(_M_CAT2ARGS_)(), M_EMPTY)(__VA_ARGS__)
#define M_CAT2ARGS(...) M_EVAL(M_IF(M_VALEN(__VA_ARGS__))(_M_CAT2ARGS, M_EMPTY)(__VA_ARGS__))

#define _M_COMPARE_() _M_COMPARE
#define _M_COMPARE(v, x, ...) v == x || M_IF(M_VALEN(__VA_ARGS__))(M_OBSTRUCT(_M_COMPARE_)(), M_ZERO)(v, __VA_ARGS__) 
#define M_COMPARE(v, ...) (M_EVAL(M_IF(M_VALEN(__VA_ARGS__))(_M_COMPARE, M_ZERO)(v, __VA_ARGS__)))

#define ASSERT(x, ...) ({ if (!M_COMPARE(x, __VA_ARGS__)) ERROR("Assert Faild!"); })
#define ERROR(...) ({ printf("Error (%s: %s): ", M_STR(__FILE__), M_STR(__LINE__)); print(__VA_ARGS__); exit(-1); })

#define unreachable() __builtin_unreachable()

typedef struct span_t {
   char *ptr; 
   size_t len;
} span_t;

static inline void print_span(span_t *span) {
   for (int i = 0; i < span->len; i++) printf("%c", span->ptr[i]);
}

#endif