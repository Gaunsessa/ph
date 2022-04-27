#ifndef DYNARR_H
#define DYNARR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define dynarr_t(type) type**

#define dyi(arr) (*(arr))

#define dy_init(type, ...)                                                                                     \
   ({                                                                                                          \
      dynarr_t(type) arr = (dynarr_t(type))_dynarr_init(sizeof(type));                                         \
      type data[] = { __VA_ARGS__ };                                                                           \
      for (int i = 0; i < sizeof(data) / sizeof(type); i++)                                                    \
         dy_push(arr, data[i]);                                                                                \
      arr;                                                                                                     \
   })                                                                                                          \

#define dy_push(arr, v)                                                                                        \
   ({                                                                                                          \
      dyi(arr)[_dy_info((arr))->index++] = (v);                                                                \
      _dynarr_grow((void **)(arr));                                                                            \
   })                                                                                                          \

#define dy_pop(arr)                                                                                            \
   ({                                                                                                          \
      __typeof__(dyi(arr)[0]) elm = dyi(arr)[--_dy_info((arr))->index];                                        \
      _dynarr_shrink((void **)(arr));                                                                          \
      elm;                                                                                                     \
   })                                                                                                          \

#define dy_remove(arr, i)                                                                                      \
   ({                                                                                                          \
      __typeof__(dyi(arr)[0]) elm = dyi(arr)[i];                                                               \
      memmove(dyi(arr) + (i), dyi(arr) + (i) + 1, (_dy_info((arr))->index - (i)) * _dy_info((arr))->elm_size); \
      _dy_info((arr))->index--;                                                                                \
      _dynarr_shrink((void **)(arr));                                                                          \
      elm;                                                                                                     \
   })                                                                                                          \

#define dy_len(arr) _dy_info((arr))->index

#define dy_free(arr)                                                                                           \
   ({                                                                                                          \
      free(_dy_info((arr)));                                                                                   \
      free(arr);                                                                                               \
   })                                                                                                          \

#define _dy_info(arr) ((dynarr_info_t *)(*(arr)) - 1)

typedef struct dynarr_info_t {
   size_t elm_size;
   size_t index;
   size_t length;
} dynarr_info_t;

static inline void **_dynarr_init(size_t size) {
   void **arr = malloc(sizeof(void *));

   *arr = (dynarr_info_t *)calloc(1, sizeof(dynarr_info_t) + sizeof(size)) + 1;

   _dy_info(arr)->elm_size = size;
   _dy_info(arr)->length   = 1;

   return arr;
}

static inline int _dynarr_resize(void **arr, int size) {
   if (arr == NULL) return -1;

   dynarr_info_t *info = _dy_info(arr);

   info->length = size;

   *arr = (dynarr_info_t *)realloc(info, sizeof(dynarr_info_t) + info->elm_size * info->length) + 1;

   return 0;
}

static inline int _dynarr_grow(void **arr) {
   if (arr == NULL) return -1;

   dynarr_info_t *info = _dy_info(arr);

   if (info->index == info->length)
      return _dynarr_resize(arr, info->length * 2);

   return 0;
}

static inline int _dynarr_shrink(void **arr) {
   if (arr == NULL) return -1;

   dynarr_info_t *info = _dy_info(arr);

   if (info->index <= info->length / 2 && info->length != 0)
      return _dynarr_resize(arr, info->length / 2);

   return 0;
}

#endif