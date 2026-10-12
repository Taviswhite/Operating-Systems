#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t sig_delivered = 0;
int alarms_delivered = 0;
time_t start_time;

void alarm_handler(int signum)
{
  printf("Hello World!\n");
  sig_delivered = 1;
}

void int_handler(int signum)
{
  time_t end_time = time(NULL);
  int total_seconds = (int)(end_time - start_time);

  printf("\nProgram terminated.\n");
  printf("Total alarms occurered: %d\n", alarms_delivered);
  printf("Total system execution time: %d seconds.\n", total_seconds);
  exit(0);
}
int main(int argc, char * argv[])
{
  start_time = time(NULL);

  signal(SIGALRM, alarm_handler);
  signal(SIGINT, int_handler);

  while(1){
    alarm(1);
    sig_delivered = 0;

    while(!sig_delivered);

    printf("Turing was right!\n");
    alarms_delivered++;
  }
  return 0;
}
