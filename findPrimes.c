    /*
    * C program to find prime numbers in a given range.
    * Also print the number of prime numbers.
    */
   #include <stdio.h>
   #include <stdlib.h>
    
   int findPrimes(int bot, int top)
   {
        int flag = 0;
        if (top < 2)
            return 0;
        if ( bot % 2 == 0)
           bot++;
       for (i = bot; i <= num2; i = i + 2)
       {
           flag = 0;
           for (j = 2; j <= i / 2; j++)
           {
               if ((i % j) == 0)
               {
                   flag = 1;
                   break;
               }
           }
           if (flag == 0)
           {
               printf("%d\n", i);
               count++;
           }
       }
       printf("Number of primes between %d & %d = %d\n", temp, num2, count);
   }