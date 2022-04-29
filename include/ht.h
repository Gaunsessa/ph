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
   size_t key_size;
   size_t value_size;

   ht_entry_t **buckets;
} ht_inner_t;

typedef struct ht_const_t {
   size_t key_size;
   size_t value_size;

   size_t amt;

   void *data;
} ht_const_t;

#define _ht_arg(_, x, ...) x
#define _ht_size(s, v) ((s) ?: (strlen((void *)(v)) + 1))
#define _ht_cmp(s, v1, v2) (s ? memcmp(v1, v2, s) : strcmp(v1, v2))

#define ht_t(k, v) struct __attribute__((__packed__)) { k key; v value; } *

#define ht_c(k, v, ...)    (void *)&(ht_const_t) { sizeof(k), sizeof(v), sizeof((struct { k key; v value; }[]) { __VA_ARGS__ }) / sizeof(struct { k key; v value; }), (void *)(struct __attribute__((__packed__)) { k key; v value; }[]) { __VA_ARGS__ }}
#define ht_c_ss(...)       (void *)&(ht_const_t) { 0, 0, sizeof((struct { char *key; char *value; }[]) { __VA_ARGS__ }) / sizeof(struct { char *key; char *value; }), (void *)(struct __attribute__((__packed__)) { char *key; char *value; }[]) { __VA_ARGS__ }}
#define ht_c_sv(v, ...)    (void *)&(ht_const_t) { 0, sizeof(v), sizeof((struct { char *key; v value; }[]) { __VA_ARGS__ }) / sizeof(struct { char *key; v value; }), (void *)(struct __attribute__((__packed__)) { char *key; v value; }[]) { __VA_ARGS__ }}
#define ht_c_vs(k, ...)    (void *)&(ht_const_t) { sizeof(k), 0, sizeof((struct { k key; char *value; }[]) { __VA_ARGS__ }) / sizeof(struct { k key; char *value; }), (void *)(struct __attribute__((__packed__)) { k key; char *value; }[]) { __VA_ARGS__ }}
#define ht_c_vv(k, v, ...) (void *)&(ht_const_t) { sizeof(k), sizeof(v), sizeof((struct { k key; v value; }[]) { __VA_ARGS__ }) / sizeof(struct { k key; v value; }), (void *)(struct __attribute__((__packed__)) { k key; v value; }[]) { __VA_ARGS__ }}

#define ht_init(k, v, ...)    _ht_init(sizeof(k), sizeof(v), (void *)_ht_arg(_, ##__VA_ARGS__, NULL))
#define ht_init_ss(...)       _ht_init(0, 0, (void *)_ht_arg(_, ##__VA_ARGS__, NULL))
#define ht_init_sv(v, ...)    _ht_init(0, sizeof(v), (void *)_ht_arg(_, ##__VA_ARGS__, NULL))
#define ht_init_vs(k, ...)    _ht_init(sizeof(k), 0, (void *)_ht_arg(_, ##__VA_ARGS__, NULL))
#define ht_init_vv(k, v, ...) _ht_init(sizeof(k), sizeof(v), (void *)_ht_arg(_, ##__VA_ARGS__, NULL))

#define ht_get(ht, k)    ((__typeof__(ht))_ht_get((ht_inner_t *)(ht), (__typeof__(k)[1]) { (k) }))->value
#define ht_get_ss(ht, k) ({ uint8_t *v = _ht_get((ht_inner_t *)(ht), (k)); (__typeof__(ht->value))(v + strlen((char *)v) + 1); })
#define ht_get_sv(ht, k) ({ uint8_t *v = _ht_get((ht_inner_t *)(ht), (k)); *(__typeof__(ht->value) *)(v + strlen((char *)v) + 1); })
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

#define ht_free(ht) _ht_free((void *)ht)

static inline int ht_hash(ht_inner_t *ht, void *key) {
   uint8_t *d = key;
    uint64_t h = 0xcbf29ce484222325;

    for (int i = 0; i < (ht->key_size ?: strlen((char *)d)); i++)
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
      if (!_ht_cmp(ht->key_size, bucket->data, key))
         return bucket->data;
   } while ((bucket = bucket->next) != NULL);

   return NULL;
}

static inline void _ht_new_bucket(ht_inner_t *ht, ht_entry_t **bucket, void *key, void *data) {
   *bucket = calloc(1, sizeof(ht_entry_t));

   (*bucket)->data = malloc(_ht_size(ht->key_size, key) + _ht_size(ht->value_size, data));

   memcpy((*bucket)->data, key, _ht_size(ht->key_size, key));
   memcpy((uint8_t *)(*bucket)->data + _ht_size(ht->key_size, key), data, _ht_size(ht->value_size, data));
}

static inline void _ht_set(ht_inner_t *ht, void *key, void *data) {
   int index = ht_hash(ht, key);

   if (ht->buckets[index] == NULL) 
      return _ht_new_bucket(ht, &ht->buckets[index], key, data);

   ht_entry_t *bucket = ht->buckets[index];

   while (true) {
      if (!_ht_cmp(ht->key_size, bucket->data, key)) {
         if (_ht_size(ht->value_size, data) != _ht_size(ht->value_size, (uint8_t *)ht->buckets[index]->data + _ht_size(ht->key_size, key)))
            bucket->data = realloc(bucket->data, _ht_size(ht->key_size, key) + _ht_size(ht->value_size, data));

         memcpy((uint8_t *)bucket->data + _ht_size(ht->key_size, key), data, _ht_size(ht->value_size, data));

         return;
      }

      if (bucket->next == NULL) {
         // if (ht->key_size == 0) printf("%s\n", key);

         return _ht_new_bucket(ht, &bucket->next, key, data);
      }

      bucket = bucket->next;
   }
}

static inline void *_ht_init(size_t key_size, size_t value_size, ht_const_t *cdata) {
   ht_inner_t *inner = malloc(sizeof(ht_inner_t));

   inner->key_size   = key_size;
   inner->value_size = value_size;

   inner->buckets    = calloc(_ht_bucket_amt(), sizeof(ht_entry_t **));

   if (cdata != NULL)
      for (int i = 0; i < cdata->amt; i++) {
         void *k = (uint8_t *)cdata->data + ((cdata->key_size ?: sizeof(char *)) + (cdata->value_size ?: sizeof(char *))) * i;
         void *v = (uint8_t *)cdata->data + ((cdata->key_size ?: sizeof(char *)) + (cdata->value_size ?: sizeof(char *))) * i + (cdata->key_size ?: sizeof(char *));

         _ht_set(
            inner, 
            cdata->key_size ? k : *(char **)k,
            cdata->value_size ? v : *(char **)v
         );
      }

   return inner;
}

static inline void _ht_free_bucket(ht_entry_t *bucket) {
   if (bucket == NULL) return;

   ht_entry_t *next = bucket->next;

   free(bucket->data);
   free(bucket);

   _ht_free_bucket(next);
}

static inline void _ht_free(ht_inner_t *ht) {
   for (int i = 0; i < _ht_bucket_amt(); i++)
      _ht_free_bucket(ht->buckets[i]);

   free(ht->buckets);
   free(ht);
}

#endif