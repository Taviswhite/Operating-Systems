#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * Executes the command "cat scores | grep Lakers".  In this quick-and-dirty
 * implementation the parent doesn't wait for the child to finish and
 * so the command prompt may reappear before the child terminates.
 *
 */

int main(int argc, char **argv)
{
  int fd1[2];
  int fd2[2];
  int pid1, pid2;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
    return 1;
  }

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  // make a pipe 

  if(pipe(fd1) == -1) {
    perror("pipe fd1 failed");
    return 1;
  }
  if(pipe(fd2) == -1) {
    perror("pipe fd2 failed");
    return 1;
  }

  pid1 = fork();
  if (pid1 < 0) {
    perror("fork pid1 failed");
    return 1;
  }

  if (pid1 == 0)
    {
      close(fd1[1]);

      pid2 = fork();
      if (pid2 < 0) {
        perror("fork pid2 failed");
        exit(1);
      }

      if (pid2 == 0)
      {
        // grandchild gets here and handles "sort"
        dup2(fd2[0], 0);
    
        // close unused halves of both pipes

        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);

        // execute sort

        execvp("sort", sort_args);
        perror("execvp sort failed");
        exit(1);
      }
    else
      {
        // parent gets here and handles "grep <argv[1]>"

        // replace standard input with read end of first pipe

        dup2(fd1[0], 0);

        // replace standard output with write end of second pipe

        dup2(fd2[1], 1);

        // close unused unput half of pipe

        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);

        // execute grep

        execvp("grep", grep_args);
        perror("execvp grep failed");
        exit(1);
      }
    }
  else
    {
      // parent gets here and handle "cat sort"
      // replace standard output with write end of first pipe

      dup2(fd1[1], 1);

      // close unused halves of both pipes

      close(fd1[0]);
      close(fd1[1]);
      close(fd2[0]);
      close(fd2[1]);

      // execute cat

      execvp("cat", cat_args);
      perror("execvp cat failed");
      return 1;
    }  
}
