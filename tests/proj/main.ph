module main

import test

t :: struct {
   x: f32
}

main :: (): = {
   x := test'vec { 32, 32 }

   a: test'print()

   ham()

   x.mul(10)
}