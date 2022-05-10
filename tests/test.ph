// Hello World

sqrt :: (x: f64): f64 = ---

vec2 :: struct {
   x: f64
   y: f64
}

impl vec2 {
   new_none :: (): vec2 = return vec2 {}
   new_ints :: (x: int, y: int): vec2 = return vec2 { (f64)x, (f64)y }
   // new :: :{new_none, new_ints}:

   mag :: (v): f64 = return sqrt(v.x * v.x + v.y * v.y)

   mul :: (v, x: f64): = {
      v.x *= x
      v.y *= x
   }
}

main :: (): int = {
   x := vec2 { x = 1.0, y = 1.0 }

   x.mul(32.0)

   return (int)x.mag()
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