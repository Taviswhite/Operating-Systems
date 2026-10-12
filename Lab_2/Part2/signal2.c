#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t sig_delivered = 0;

void handler(int signum)
{ //signal handler
  printf("Hello World!\n");
  sig_delivered = 1;
}

int main(int argc, char * argv[])
{
  signal(SIGALRM,handler); //register handler to handle SIGALRM
  
  while(1){
    alarm(5);
    sig_delivered = 0;

    while(!sig_delivered);
    
    printf("Turing was right!\n");
  }
  return 0; 
}
