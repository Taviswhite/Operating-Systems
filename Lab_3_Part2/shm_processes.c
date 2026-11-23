#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <time.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <sys/wait.h>

#ifndef SLEEP
#define SLEEP(s) sleep(s)
#endif

void  ParentProcess(int SharedMem[]);
void  ClientProcess(int SharedMem[]);

int  main(int  argc, char *argv[])
{
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;
     int    status;

     if (argc != 5) {
          printf("Use: %s #1 #2 #3 #4\n", argv[0]);
          exit(1);
     }

     /* Two shared ints: ShmPTR[0] = BankAccount, ShmPTR[1] = Turn */
     ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of two integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");

     ShmPTR[0] = 0;   /* BankAccount */
     ShmPTR[1] = 0;   /* Turn        */
     printf("Server has initialized BankAccount = %d and Turn = %d\n",
            ShmPTR[0], ShmPTR[1]);

     printf("Server is about to fork a child process...\n");
     fflush(stdout);
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }
     else if (pid == 0) {
          ClientProcess(ShmPTR);
          exit(0);
     }

     ParentProcess(ShmPTR);

     wait(&status);
     printf("Server has detected the completion of its child...\n");
     shmdt((void *) ShmPTR);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(0);
}

/* Dear Old Dad: deposits money */
void  ParentProcess(int SharedMem[])
{
     int i, account, balance;
     srand(time(NULL) ^ getpid());

     for (i = 0; i < 25; i++) {
          SLEEP(rand() % 6);                 /* sleep 0 - 5 seconds */
          account = SharedMem[0];            /* copy BankAccount */
          while (SharedMem[1] != 0)          /* strict alternation */
               ;
          if (account <= 100) {
               balance = rand() % 101;       /* 0 - 100 */
               if (balance % 2 == 0) {
                    account += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
               } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
               }
          } else {
               printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
          }
          fflush(stdout);
          SharedMem[0] = account;            /* copy back to BankAccount */
          SharedMem[1] = 1;                  /* Turn = 1 */
     }
}

/* Poor Student: withdraws money */
void  ClientProcess(int SharedMem[])
{
     int i, account, balance;
     srand(time(NULL) ^ (getpid() << 8));

     for (i = 0; i < 25; i++) {
          SLEEP(rand() % 6);                 /* sleep 0 - 5 seconds */
          account = SharedMem[0];            /* copy BankAccount */
          while (SharedMem[1] != 1)          /* strict alternation */
               ;
          balance = rand() % 51;             /* 0 - 50 */
          printf("Poor Student needs $%d\n", balance);
          if (balance <= account) {
               account -= balance;
               printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
          } else {
               printf("Poor Student: Not Enough Cash ($%d)\n", account);
          }
          fflush(stdout);
          SharedMem[0] = account;            /* copy back to BankAccount */
          SharedMem[1] = 0;                  /* Turn = 0 */
     }
}
