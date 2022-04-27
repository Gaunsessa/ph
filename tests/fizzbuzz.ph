// Hello World

main :: (argc: int, argv: ^^char): int = {
   for i: int = 1; i <= 30; i += 1 {
      if i % 3 == 0 -> printf("fizz")
      if i % 5 == 0 -> printf("buzz")

      if i % 3 != 0 && i % 5 != 0 -> printf("%d", i)

      printf("\n")
   }

   return 0
}