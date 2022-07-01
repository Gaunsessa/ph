module main

vec :: struct {
   x: f32
   y: f32
}

impl vec {
   mul :: (v, x: f32): vec = {
      // v.x *= x
      // v.y *= x

      // return v
   }
}

main :: (): int = {
   x := vec'{ 32.0, 32.0 }

   x.mul(32.0).mul(32.0)

   return 0
}