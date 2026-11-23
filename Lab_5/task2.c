#include <stdio.h>
#include <stdlib.h>

/* IMPLEMENT ME: Declare your functions here */
int add (int a, int b);
int subtract (int a, int b);
int multiply (int a, int b);
int divide (int a, int b);
int quit (int a, int b);


int main (void)
{
  /* IMPLEMENT ME: Insert your algorithm here */
  int a = 6, b = 3, x;
  char choice;

  /* Table of function pointers, indexed by the user's choice */
  int (*operations[5]) (int, int) = { add, subtract, multiply, divide, quit };

  printf ("Operand 'a' : %d | Operand 'b' : %d\n", a, b);
  printf ("Specify the operation to perform (0 : add | 1 : subtract | 2 : Multiply | 3 : divide | 4 : exit): ");
  scanf (" %c", &choice);

  /* '0'..'4' minus '0' gives index 0..4 -- no conditionals needed */
  x = operations[choice - '0'] (a, b);
  printf ("x = %d\n", x);

  return 0;
}

/* IMPLEMENT ME: Define your functions here */
int add (int a, int b) { printf ("Adding 'a' and 'b'\n"); return a + b; }
int subtract (int a, int b) { printf ("Subtracting 'b' from 'a'\n"); return a - b; }
int multiply (int a, int b) { printf ("Multiplying 'a' and 'b'\n"); return a * b; }
int divide (int a, int b) { printf ("Dividing 'a' by 'b'\n"); return a / b; }
int quit (int a, int b) { (void) a; (void) b; exit (0); }
