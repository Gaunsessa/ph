#include <stdio.h>
#include <dlfcn.h>
#include <math.h>

#define _M_CAT(x, y) x##y
#define M_CAT(x, y) _M_CAT(x, y)

#define INIT void M_CAT(init_, __COUNTER__)(void)

INIT {
   printf("Joe Mama!\n");
}

int main(int argc, char **argv) {
   char fname[sizeof("init_") + 10];

   for (int i = 0;; i++) {
      sprintf(fname, "init_%d", i);

      void (*func)(void) = dlsym(RTLD_DEFAULT, fname);

      if (func != NULL) func();
      else break;
   }

   return 0;
}

INIT {
   printf("Joe Mama2!\n");
}