Entity :: struct {
   pos: vec2

   get_pos :: (e: Entity): vec2 = return e.pos
}

impl Entity {
   kill :: (e: Entity): = {}
}

Cow :: struct {
   use ent: Entity

   get_pos :: (e: Cow): vec2 = return e.pos * 2
}

cum :: (e: Entity): = {
   if e.get_pos().x > 3 -> printf("cum")
}

main :: (): int {
   c := Cow { pos = { 32, 32 } }
   c = Cow { ent = { pos = { 32, 32 } } }

   c.get_pos() // 64, 64
   c.ent.get_pos() // 32, 32

   c.kill()

   // c.cum() is invalid!
   // cum(c) is valid!
}