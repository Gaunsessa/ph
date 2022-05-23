module test

import main

vec :: struct {
   x: f32
   y: f32
}

impl vec {
   mul :: (v, x: f32): = {
      v.x *= x;
      v.y *= x;
   }
}

test_func :: (x: int): int = {
   main.main()

   return test_func(32)
}