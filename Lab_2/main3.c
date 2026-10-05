#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_ITERATIONS 30   //max child loops
#define MAX_SLEEP_SECS 10   //max child sleep
#define NUM_CHILDREN    2   //number of children the parent creates

void ChildProcess(void) {
    pid_t pid = getpid();
    pid_t ppid = getppid();

    srandom((unsigned int) time(NULL) ^ (unsigned int) pid);

    int iterations = (int)(random() % MAX_ITERATIONS) + 1;

    for (int i = 0; i < iterations; i++) {
        printf("Child Pid: %d is going to sleep!\n", pid);
        fflush(stdout);

        int sleep_secs = (int)(random() % MAX_SLEEP_SECS) + 1;
        sleep((unsigned int) sleep_secs);
    
        printf("Child Pid: %d is awake\nWhere is my Parent: %d?\n", pid, ppid);  
        fflush(stdout);

    }
   
    exit(0);

}

void ParentProcess(void) {
for (int i = 0; i < NUM_CHILDREN; i++){
    int status;
    pid_t finished_pid = wait(&status);
    
    if (finished_pid > 0) {
        printf("Child Pid: %d has completed\n", finished_pid);
        fflush(stdout);
        }
    }
}

int main(void) {
    pid_t pid1 = fork();
    
    if (pid1 < 0) {
        fprintf(stderr, "fork() failed for first child\n");
        exit(1);
    }
    if (pid1 == 0) {
        ChildProcess();
    }

    pid_t pid2 = fork();
    
    if (pid2 < 0) {
        fprintf(stderr,"fork() failed for second child\n");
        exit(1);
    }

    if (pid2 == 0) {
        ChildProcess();
    }
        
    ParentProcess();
    return 0;
    
}

