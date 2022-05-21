Entity :: struct {
   x: f64
   y: f64
}

impl Entity {
   kill :: (e): = {}
}

_entity_kill :: (e: Entity): = {}

func :: (): = {}

main :: (): int = {
   c := Entity { 32, 32 }

   c.kill()

   func()

   // _entity_kill(&c)

   // c.cum() is invalid!
   // cum(c) is valid!
}