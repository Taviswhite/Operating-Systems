#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t sig_delivered = 0;

void handler(int signum)
{ //signal handler
  printf("Hello World!\n");
  sig_delivered = 1; // update flag
}

int main(int argc, char * argv[])
{
  signal(SIGALRM,handler); //register handler to handle SIGALRM
  alarm(5); //Schedule a SIGALRM for 5 seconds
  
  while(!sig_delivered); //busy wait for signal to be delivered
  
  printf("Turing was right!\n");
  return 0; //never reached
}
