#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>
#include <stdarg.h>

#ifndef __EMSCRIPTEN__
#define __NORM "\x1B[0m"
#define __BLUE "\x1B[34m"
#define __MAGE "\x1B[35m"
#else
#define __NORM
#define __BLUE
#define __MAGE
#endif

#define __print_fmt(x) _Generic((x),                                           \
   signed char:        __BLUE "%d ",    unsigned char:         __BLUE "%i ",   \
   signed short:       __BLUE "%hi ",   unsigned short:        __BLUE "%hu ",  \
   signed int:         __BLUE "%d ",    unsigned int:          __BLUE "%u ",   \
   signed long:        __BLUE "%ld ",   unsigned long:         __BLUE "%lu ",  \
   signed long long:   __BLUE "%lld ",  unsigned long long:    __BLUE "%llu ", \
   float:              __MAGE "%f ",    double:                __MAGE "%f ",   \
   long double:        __MAGE "%Lf ",   char *:                __NORM "%s ",   \
   char:               __BLUE "%d ",    default:               NULL )          \

#define __print(x) if (__print_fmt(x) != NULL) printf(__print_fmt(x), x)

#define __va_print(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, ...)  \
   ({                                                                                                                                                                               \
      __print(_0);  __print(_1);  __print(_2);  __print(_3);  __print(_4);  __print(_5);  __print(_6);  __print(_7);  __print(_8);  __print(_9);  __print(_10);                     \
      __print(_11); __print(_12); __print(_13); __print(_14); __print(_15); __print(_16); __print(_17); __print(_18); __print(_19); __print(_20); __print(_21);                     \
      __print(_22); __print(_23); __print(_24); __print(_25); __print(_26); __print(_27); __print(_28); __print(_29); __print(_30); __print(_31); __print(_32);                     \
                                                                                                                                                                                    \
      printf(__NORM "\n");                                                                                                                                                          \
    })                                                                                                                                                                              \

#define print(...) __va_print(__VA_ARGS__, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)

#define eprint(...) ({ print(__VA_ARGS__); exit(-1); })

#endif