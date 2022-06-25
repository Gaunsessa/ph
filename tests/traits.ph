module main

st :: struct {
   x: int
}

coom :: trait {
   output: type

   coom: (s): output
}

impl st {
   coom :: (s): = {

   }
}

test :: (x: st): st = {
   // x := st'{}
   x.coom()

   return x
}
