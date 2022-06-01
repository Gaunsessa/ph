module main

// https://www.csie.ntu.edu.tw/~b93501005/slide5.pdf
// Give all scopes a unique id
// Give all variables in that scope the scope id
// When looking up a variable look for the closest scope ided var

// 0
x := 10

main :: (): = {
   // 1
   x := 10

   {
      // 2
      x := 10

      {
         // 3
      }

   }

   {
      // 4
   }
}

test :: (): = {
   
}
