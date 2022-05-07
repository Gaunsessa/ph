#ifndef HT_H
#define HT_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct ht_entry_t {
   void *data;

   struct ht_entry_t *next;
} ht_entry_t;

typedef struct ht_inner_t {
   bool key_str;
   size_t key_size;

   bool value_str;
   size_t value_size;

   ht_entry_t **buckets;
} ht_inner_t;

typedef struct ht_const_t {
   bool key_str;
   size_t key_size;

   bool value_str;
   size_t value_size;

   size_t amt;

   void *data;
} ht_const_t;

#define _ht_arg(_, x, ...) x
// #define _ht_size(s, v) ((s) ?: (strlen((void *)(v)) + 1))
#define _ht_cmp(s, v1, v2) (s ? memcmp(v1, v2, s) : strcmp(v1, v2))

#define ht_t(k, v) struct __attribute__((__packed__)) { k key; v value; } *

#define ht_c(k, v, ...)    (void *)&(ht_const_t) { false, sizeof(k), false, sizeof(v), sizeof((struct { k key; v value; }[]) { __VA_ARGS__ }) / sizeof(struct { k key; v value; }), (void *)(struct __attribute__((__packed__)) { k key; v value; }[]) { __VA_ARGS__ }}
#define ht_c_ss(k, v, ...) (void *)&(ht_const_t) { true, sizeof(k*), true, sizeof(v*), sizeof((struct { k *key; v *value; }[]) { __VA_ARGS__ }) / sizeof(struct { k *key; v *value; }), (void *)(struct __attribute__((__packed__)) { k *key; v *value; }[]) { __VA_ARGS__ }}
#define ht_c_sv(k, v, ...) (void *)&(ht_const_t) { true, sizeof(k*), false, sizeof(v), sizeof((struct { k *key; v value; }[]) { __VA_ARGS__ }) / sizeof(struct { k *key; v value; }), (void *)(struct __attribute__((__packed__)) { k *key; v value; }[]) { __VA_ARGS__ }}
#define ht_c_vs(k, v, ...) (void *)&(ht_const_t) { false, sizeof(k), true, sizeof(v*), sizeof((struct { k key; v *value; }[]) { __VA_ARGS__ }) / sizeof(struct { k key; v *value; }), (void *)(struct __attribute__((__packed__)) { k key; v *value; }[]) { __VA_ARGS__ }}
#define ht_c_vv(k, v, ...) (void *)&(ht_const_t) { false, sizeof(k), false, sizeof(v), sizeof((struct { k key; v value; }[]) { __VA_ARGS__ }) / sizeof(struct { k key; v value; }), (void *)(struct __attribute__((__packed__)) { k key; v value; }[]) { __VA_ARGS__ }}

#define ht_init(k, v, ...)    _ht_init(false, sizeof(k), false, sizeof(v), (void *)_ht_arg(_, ##__VA_ARGS__, NULL))
#define ht_init_ss(k, v, ...) _ht_init(true, sizeof(k), true, sizeof(v), (void *)_ht_arg(_, ##__VA_ARGS__, NULL))
#define ht_init_sv(k, v, ...) _ht_init(true, sizeof(k), false, sizeof(v), (void *)_ht_arg(_, ##__VA_ARGS__, NULL))
#define ht_init_vs(k, v, ...) _ht_init(false, sizeof(k), true, sizeof(v), (void *)_ht_arg(_, ##__VA_ARGS__, NULL))
#define ht_init_vv(k, v, ...) _ht_init(false, sizeof(k), false, sizeof(v), (void *)_ht_arg(_, ##__VA_ARGS__, NULL))

#define ht_get(ht, k)    ((__typeof__(ht))_ht_get((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }))->value
#define ht_get_ss(ht, k) ({ uint8_t *v = _ht_get((ht_inner_t *)(ht), (k)); (__typeof__(ht->value))(v + _ht_size((ht_inner_t *)ht, true, v)); })
#define ht_get_sv(ht, k) ({ uint8_t *v = _ht_get((ht_inner_t *)(ht), (k)); *(__typeof__(ht->value) *)(v + _ht_size((ht_inner_t *)ht, true, v)); })
#define ht_get_vs(ht, k) ({ uint8_t *v = _ht_get((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }); (__typeof__(ht->value))(v + sizeof(ht->key)); })
#define ht_get_vv(ht, k) ((__typeof__(ht))_ht_get((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }))->value

#define ht_set(ht, k, v)    _ht_set((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }, (__typeof__(v)[1]) { (v) })
#define ht_set_ss(ht, k, v) _ht_set((ht_inner_t *)(ht), (k), (v))
#define ht_set_sv(ht, k, v) _ht_set((ht_inner_t *)(ht), (k), (__typeof__(v)[1]) { (v) })
#define ht_set_vs(ht, k, v) _ht_set((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }, (v))
#define ht_set_vv(ht, k, v) _ht_set((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }, (__typeof__(v)[1]) { (v) })

#define ht_exists(ht, k)    (_ht_get((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }) != NULL)
#define ht_exists_ss(ht, k) (_ht_get((ht_inner_t *)(ht), (k)) != NULL)
#define ht_exists_sv(ht, k) (_ht_get((ht_inner_t *)(ht), (k)) != NULL)
#define ht_exists_vs(ht, k) (_ht_get((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }) != NULL)
#define ht_exists_vv(ht, k) (_ht_get((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }) != NULL)

#define ht_free(ht) _ht_free((void *)ht, NULL)
#define ht_free_func(ht, func) _ht_free((void *)ht, func)

static inline size_t _ht_size(ht_inner_t *ht, bool key, void *data) {
   size_t elm_size = key ? ht->key_size : ht->value_size;

   if (key ? ht->key_str : ht->value_str) {
      uint8_t *cdata = data;
      uint8_t cmp[elm_size];
      memset(cmp, 0, elm_size);

      int i = 0;

      while (true) {
         if (!memcmp(cdata + i * elm_size, cmp, elm_size)) break;

         i++;
      }

      return (i + 1) * elm_size;
   } else return elm_size;
}

static inline int ht_hash(ht_inner_t *ht, void *key) {
   uint8_t *d = key;
   uint64_t h = 0xcbf29ce484222325;

   for (int i = 0; i < _ht_size(ht, true, key); i++)
       h = (h * 0x100000001b3) ^ d[i];

#ifdef HT_BUCKET_AMT
   return h % HT_BUCKET_AMT;
#else
   return h % 32;
#endif
}

static inline size_t _ht_bucket_amt() {
   return
#ifdef HT_BUCKET_AMT
   HT_BUCKET_AMT;
#else
   32;
#endif
}

static inline void *_ht_get(ht_inner_t *ht, void *key) {
   ht_entry_t *bucket = ht->buckets[ht_hash(ht, key)];   

   if (bucket == NULL) return NULL;

   do {
      if (_ht_size(ht, true, key) == _ht_size(ht, true, bucket->data) && !memcmp(key, bucket->data, _ht_size(ht, true, key)))
         return bucket->data;
   } while ((bucket = bucket->next) != NULL);

   return NULL;
}

static inline void _ht_new_bucket(ht_inner_t *ht, ht_entry_t **bucket, void *key, void *data) {
   *bucket = calloc(1, sizeof(ht_entry_t));

   (*bucket)->data = malloc(_ht_size(ht, true, key) + _ht_size(ht, false, data));

   memcpy((*bucket)->data, key, _ht_size(ht, true, key));
   memcpy((uint8_t *)(*bucket)->data + _ht_size(ht, true, key), data, _ht_size(ht, false, data));
}

static inline void _ht_set(ht_inner_t *ht, void *key, void *data) {
   int index = ht_hash(ht, key);

   if (ht->buckets[index] == NULL) 
      return _ht_new_bucket(ht, &ht->buckets[index], key, data);

   ht_entry_t *bucket = ht->buckets[index];

   while (true) {
      if (_ht_size(ht, true, key) == _ht_size(ht, true, bucket->data) && !memcmp(key, bucket->data, _ht_size(ht, true, key))) {
         if (_ht_size(ht, false, data) != _ht_size(ht, false, (uint8_t *)ht->buckets[index]->data + _ht_size(ht, true, key)))
            bucket->data = realloc(bucket->data, _ht_size(ht, true, key) + _ht_size(ht, false, data));

         memcpy((uint8_t *)bucket->data + _ht_size(ht, true, key), data, _ht_size(ht, false, data));

         return;
      }

      if (bucket->next == NULL) {
         // if (ht->key_size == 0) printf("%s\n", key);

         return _ht_new_bucket(ht, &bucket->next, key, data);
      }

      bucket = bucket->next;
   }
}

static inline void *_ht_init(bool key_str, size_t key_size, bool value_str, size_t value_size, ht_const_t *cdata) {
   ht_inner_t *inner = malloc(sizeof(ht_inner_t));

   inner->key_str    = key_str;
   inner->key_size   = key_size;

   inner->value_str  = value_str; 
   inner->value_size = value_size;

   inner->buckets    = calloc(_ht_bucket_amt(), sizeof(ht_entry_t **));

   if (cdata != NULL)
      for (int i = 0; i < cdata->amt; i++) {
         void *k = (uint8_t *)cdata->data + (cdata->key_size + cdata->value_size) * i;
         void *v = (uint8_t *)k + cdata->key_size;

         _ht_set(
            inner, 
            cdata->key_str ? *(void **)k : k,
            cdata->value_str ? *(void **)v : v
         );
      }

   return inner;
}

static inline void _ht_free_bucket(ht_inner_t *ht, ht_entry_t *bucket, void (*func)(void *)) {
   if (bucket == NULL) return;

   ht_entry_t *next = bucket->next;

   if (func != NULL) func(bucket->data + _ht_size(ht, true, bucket->data));
   free(bucket->data);
   free(bucket);

   _ht_free_bucket(ht, next, func);
}

static inline void _ht_free(ht_inner_t *ht, void (*func)(void *)) {
   for (int i = 0; i < _ht_bucket_amt(); i++)
      _ht_free_bucket(ht, ht->buckets[i], func);

   free(ht->buckets);
   free(ht);
}

#endif