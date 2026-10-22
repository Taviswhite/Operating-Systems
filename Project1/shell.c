#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128
#define FG_TIMEOUT 10

char prompt[MAX_COMMAND_LINE_LEN] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;

static pid_t fg_pid = -1;

void sigint_handler(int sig) {
  (void)sig;
  if (fg_pid > 0) {
    kill(fg_pid,SIGINT);
  }
  printf("\n");
  fflush(stdout);
}

void sigalrm_handler(int sig) {
  (void)sig;
  if (fg_pid > 0) {
    printf("\nProcess %d exceeded %d second time limit. Terminating.\n", fg_pid, FG_TIMEOUT);
    kill(fg_pid, SIGKILL);
  }
}

// TODO 0 Modify the prompt to print the current working directory
void update_prompt() {
  char cwd[MAX_COMMAND_LINE_LEN - 2];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    snprintf(prompt, sizeof(prompt), "%s> ", cwd);
  } else {
    perror("getcwd failed");
    snprintf(prompt, sizeof(prompt), "> ");
  }
}

// TODO 1 Tokenize the command line input (split it on whitespace)
int tokenize(char *line, char **args) {
  int argc = 0;
  char *tok = strtok(line, delimiters);
  while (tok != NULL && argc < MAX_COMMAND_LINE_ARGS - 1) {
    if (tok[0] == '$') {
      char *val = getenv(tok + 1);
      args[argc++] = (val != NULL) ? val : "";
    } else {
      args[argc++] = tok;
    }
    tok = strtok(NULL, delimiters);
  }
  args[argc] = NULL;
  return argc;
}

int is_builtin(const char *cmd) {
  return (strcmp(cmd, "cd") == 0 || strcmp(cmd, "pwd") == 0 ||
          strcmp(cmd, "echo") == 0 || strcmp(cmd, "exit") == 0 ||
          strcmp(cmd, "env") == 0 || strcmp(cmd, "setenv") == 0);
}

// TODO 2 Implement Built-In Commands
void run_builtin(char **args, int argc) {
  if (strcmp(args[0], "cd") == 0) {
    const char *path = (argc > 1) ? args[1] : getenv("HOME");
    if (path == NULL) path = "/";
    if (chdir(path) != 0) {
        perror("cd failed");
    }
  } 
  else if (strcmp(args[0], "pwd") == 0) {
    char cwd[MAX_COMMAND_LINE_LEN];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd failed");
    }
  }
  else if (strcmp(args[0], "echo") == 0) {
    for (int i = 1; i < argc; i++) {
      printf("%s", args[i]);
      if ( i < argc - 1) printf(" ");
    }
    printf("\n");
  }
  else if (strcmp(args[0], "env") == 0) {
    if (argc == 1) {
      for (char **e = environ; *e != NULL; e++) {
        printf("%s\n", *e);
      }
    } else {
      char *val = getenv(args[1]);
      if (val != NULL) printf("%s\n", val);
    }
  }
  else if (strcmp(args[0], "setenv") == 0) {
    if (argc < 2) return;
    char *eq = strchr(args[1], '=');
    if (eq != NULL) {
      *eq = '\0';
      setenv(args[1], eq + 1, 1);
    } else if (argc >= 3){
      setenv(args[1], args[2], 1);
    }
  }
  else if (strcmp(args[0], "exit") == 0) {
    exit(0);
  }
}

// TODO 3 Create a child process which will execute the command line input
// TODO 4 The parent process should wait for the child to complete unless its a background process
void execute_external(char **args, int argc, int background) {
  char *infile = NULL, *outfile = NULL;
  char *clean_args[MAX_COMMAND_LINE_ARGS];
  int clean_argc = 0;

  for(int i = 0; i < argc; i++) {
    if (strcmp(args[i], "<") == 0 && i + 1 < argc) {
        infile = args[++i];
    } else if (strcmp(args[i], ">") == 0 && i + 1 < argc) {
        outfile = args[++i];
    } else {
      clean_args[clean_argc++] = args[i];
    }
  }
  clean_args[clean_argc] = NULL;

  if (clean_argc == 0) return;

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    return;
  }

  if (pid == 0) {
    // child process
    signal(SIGINT, SIG_DFL);

    if (infile != NULL) {
      int fd = open(infile, O_RDONLY);
      if (fd < 0) {
          perror("open infile failed");
          exit(1);
      }
      dup2(fd, STDIN_FILENO);
      close(fd);
    }

    if (outfile != NULL) {
      int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd < 0) {
        perror("open outfile failed");
        exit(1);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }

    execvp(clean_args[0], clean_args);
    fprintf(stderr, "execvp() failed: %s\n", strerror(errno));
    printf("An error occurred.\n");
    exit(1);
  } else {
    // parent process
    if (background) {
      printf("[background pid %d]\n", pid);
    } else {
      fg_pid = pid;
      alarm(FG_TIMEOUT);
      int status;
      waitpid(pid, &status, 0);
      alarm(0);
      fg_pid = -1;
    }
  }
}

// extra credit
void execute_pipe(char *left_line, char *right_line) {
  char *left_args[MAX_COMMAND_LINE_ARGS];
  char *right_args[MAX_COMMAND_LINE_ARGS];

  int left_argc = tokenize(left_line, left_args);
  int right_argc = tokenize(right_line, right_args);
  if (left_argc == 0 || right_argc == 0) return;

  int fd[2];
  if (pipe(fd) == -1) {
    perror("pipe failed");
    return;
  }

  pid_t p1 = fork();
  if (p1 < 0) {
    perror("fork failed");
    return;
  }
  if (p1 == 0) {
    signal(SIGINT, SIG_DFL);
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);
    execvp(left_args[0], left_args);
    fprintf(stderr, "execvp() failed: %s\n", strerror(errno));
    exit(1);
  }

  pid_t p2 = fork();
  if (p2 < 0) {
    perror("fork failed");
    return;
  }
  if (p2 == 0) {
    signal(SIGINT, SIG_DFL);
    close(fd[1]);
    dup2(fd[0], STDOUT_FILENO);
    close(fd[0]);
    execvp(left_args[0], left_args);
    fprintf(stderr, "execvp() failed: %s\n", strerror(errno));
    exit(1);
  }

  close(fd[0]);
  close(fd[1]);
  waitpid(p1, NULL, 0);
  waitpid(p2, NULL, 0);
}

int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
  
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];

    signal(SIGINT, sigint_handler);
    signal(SIGALRM, sigalrm_handler);
    	
    while (true) {

        update_prompt();
      
        do{ 
            // Print the shell prompt.
            printf("%s", prompt);
            fflush(stdout);

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
        
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
 
        }while(command_line[0] == 0x0A);  // while just ENTER pressed
        
      
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }
        
        command_line[strlen(command_line) - 1] = '\0';

        strncpy(cmd_bak, command_line, MAX_COMMAND_LINE_LEN - 1);
        cmd_bak[MAX_COMMAND_LINE_LEN - 1] = '\0';

        if (strlen(cmd_bak) == 0) continue;

        int background = 0;
        char *amp = strrchr(cmd_bak, '&');
        if (amp != NULL) {
          char *p = amp + 1;
          while (*p == ' ') p++;
          if (*p == '\0') {
            background = 1;
            *amp = '\0';
          }
        }

        char *pipe_pos = strchr(cmd_bak, '|');
        if (pipe_pos != NULL) {
          char left[MAX_COMMAND_LINE_LEN], right[MAX_COMMAND_LINE_LEN];
          *pipe_pos = '\0';
          strncpy(left, cmd_bak, sizeof(left) -1);
          left[sizeof(left) - 1] = '\0';
          strncpy(right, pipe_pos + 1, sizeof(right) - 1);
          right[sizeof(right) - 1] = '\0';
          execute_pipe(left, right);
          continue;
        }

        // 1. Tokenize the command line input
        int argc = tokenize(cmd_bak, arguments);
        if (argc == 0) continue;

        // 2. Implement built-in Commands
        if (is_builtin(arguments[0])) {
          run_builtin(arguments, argc);
          continue;
        }

        execute_external(arguments, argc, background);

    
        // 3. Create a child process which will execute the command line input

  
        // 4. The parent process should wait for the child to complete unless its a background process
      
      
        // Hints (put these into Google):
        // man fork
        // man execvp
        // man wait
        // man strtok
        // man environ
        // man signals
        
        // Extra Credit
        // man dup2
        // man open
        // man pipes
    }
    // This should never be reached.
    return -1;
}
