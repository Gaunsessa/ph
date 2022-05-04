// Hello World

puts :: (str: ^u8): int = ---

vec2 :: struct {
   x: f32
   y: f32
}

main :: (): int = {
   puts((^u8)"ham")

   inner: vec2
   a: ^vec2 = &inner
   a.x = 32.0
   a.y = 32.0

   x := a.x + 32

   return (int)x
}