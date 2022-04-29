// Hello World

a :: alias int
b :: alias a

t2 :: (x: int): int = {}

test :: (x: a, y: u64): int = {
   return x * 2
}

main :: (argc: int, argv: ^^char): int = {
   c :: alias b

   x: c = 3
   y: u64

   test(x, y)

   return x
}