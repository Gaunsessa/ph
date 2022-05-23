module main
// Hello World

log10 :: (x: f64): f64 = ---
ceil :: (x: f64): f64 = ---
floor :: (x: f64): f64 = ---
pow :: (x: f64, y: f64): f64 = ---

puts :: (str: ^u8): int = ---
putchar :: (c: int): int = ---

putnum :: (num: int): = {
   f := floor((f64)num / pow(10.0, ceil(log10((f64)num)) - 1))
   s := (f64)num - pow(10.0, ceil(log10((f64)num)) - 1) * floor((f64)num / pow(10.0, ceil(log10((f64)num)) - 1))

   if num == 1 -> putchar(49)
   else putchar((int)f + 48);

   if ceil(log10(s)) > 0 -> putnum((int)s)
}

main :: (): int = {
   for i := 1; i < 100; i += 1 {
      if i % 3 == 0 -> puts((^u8)"fizz")
      if i % 5 == 0 -> puts((^u8)"buzz")

      if i % 3 != 0 && i % 5 != 0 -> {
         putnum(i)
         putchar(10)
      }
   }

   return 0
}