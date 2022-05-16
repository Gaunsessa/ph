// package main

// import test.das

sqrt :: (x: f64): f64 = ---

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
   else putchar(10)
}

vec2 :: struct {
   x: f64
   y: f64
}

impl vec2 {
   // new_none :: (): vec2 = return vec2 {}
   // new_ints :: (x: int, y: int): vec2 = return vec2 { (f64)x, (f64)y }
   // new :: :{new_none, new_ints}:

   mag :: (v): f64 = return sqrt(v.x * v.x + v.y * v.y)

   mul :: (v, x: f64): ^vec2 = {
      v.x *= x
      v.y *= x
   }

   div :: (v, x: f64): ^vec2 = {
      v.x /= x
      v.y /= x

      return v
   }

   norm :: (v): ^vec2 = { v.div(v.mag()); return v }
}

main :: (): int = {
   x := vec2 { x = 12.0, y = 48.0 }
   v := &x

   // x.norm()

   x.norm().mul(100)

   // x,new()

   // test(32, test())

   // 32test()

   // x = x * x
   // x = vec2_astrix(&x, &x)

   putnum((int)v.x)
   putnum((int)v.y)

   return 0
}

// Hello World

// test_func_sqrt :: (x: f64): f64 = ---

// test_type_vec2 :: struct {
//    x: f64
//    y: f64
// }

// impl test_type_vec2 {
//    test_impl_vec2_mag :: (self): f64 = return test_func_sqrt(self.x * self.x + self.y * self.y)
// }

// test_func_main :: (): int = {
//    x: test_type_vec2
//    x.x = 32

//    return (int)x.test_impl_vec2_mag()
// }