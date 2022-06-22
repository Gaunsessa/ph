module main

// // https://www.csie.ntu.edu.tw/~b93501005/slide5.pdf
// // Give all scopes a unique id
// // Give all variables in that scope the scope id
// // When looking up a variable look for the closest scope ided var

vec :: struct {
   x: f32
   y: f32
}

// vec2 :: alias vec

main :: (): = {
   x := frog((int)frog(32))

   b := vec'{ 0.0, 0.0 }
}

frog :: (x: int): f32 = {
   // y := x * 2

   return x
}