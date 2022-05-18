package main

import "core:fmt"

test :: proc() -> (x: int) {
   x = 0

   defer fmt.println(x)
   defer x = 10

   return
}

main :: proc() {
   fmt.println(test())
}