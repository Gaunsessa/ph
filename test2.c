#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>

#include "include/ht.h"

int main() {
   setlocale(LC_ALL, "");

   ht_t(wchar_t *, wchar_t *) hatb = ht_c_ss(wchar_t, wchar_t, { L"alias", L"alias" }, { L"argc: int, arg", L"43" });
   hatb = ht_init_ss(wchar_t, wchar_t, hatb);

   ht_set_ss(hatb, L"😳", L"😳😳😳😳😳");

   printf("%d\n", ht_exists_ss(hatb, L"argc: int, arg"));

   return 0;
}