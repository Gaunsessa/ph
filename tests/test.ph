test :: (a: int): int = {
   if a == 10 -> return a
   else return test(a)
}