// C program to demonstrate use of fork() and pipe() 
#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
  
int main() 
{ 
    // We use two pipes 
    // First pipe to send input string from parent 
    // Second pipe to send concatenated string from child 
  
    int fd1[2];  // Used to store two ends of first pipe 
    int fd2[2];  // Used to store two ends of second pipe 
  
    char fixed_str[] = "howard.edu";
    char fixed_str2[] = "gobison.org"; 
    char input_str[100]; 
    pid_t p; 
  
    if (pipe(fd1)== -1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
    if (pipe(fd2)== -1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
  
    printf("Other string is: %s\n", fixed_str);
    printf("Input : ");
    fflush(stdout);
    if(fgets(input_str, sizeof(input_str), stdin) == NULL)
      return 1;
    input_str[strcspn(input_str, "\n")] = '\0';

    p = fork(); 
  
    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed" ); 
        return 1; 
    } 
  
    // Parent process 
    else if (p > 0) 
    { 
  
        close(fd1[0]);  // Close reading end of pipes 
        close(fd2[1]);
  
        // Write input string and close writing end of first 
        // pipe. 
        if(write(fd1[1], input_str, strlen(input_str)+1) == -1)
        {
          fprintf(stderr, "write Failed\n");
          return 1;
        } 
        close(fd1[1]);
        
        
        char result[300];
	      ssize_t n = read(fd2[0], result, sizeof(result) - 1);
	      close(fd2[0]);
  
        // Wait for child to print the concatenated string 
        wait(NULL); 

        if(n <= 0)
        {
          fprintf(stderr, "Read Failed\n");
          return 1;
        }
        result[n] = '\0';

        strncat(result, fixed_str2, sizeof(result) - strlen(result) - 1);
	      printf("Output : %s\n", result);
    } 
  
    // child process 
    else
    { 
        close(fd1[1]);  // Close writing end of first pipes 
        close(fd2[0]); 
      
        // Read a string using first pipe 
        char concat_str[300]; 
        ssize_t n = read(fd1[0], concat_str, 100);
	      close(fd1[0]); 
        if (n <= 0)
        {
          fprintf(stderr, "Read Failed\n");
          exit(1);
        }
        concat_str[n] = '\0';

        strncat(concat_str, fixed_str, sizeof(concat_str) - strlen(concat_str) - 1);
        printf("Output : %s\n", concat_str);

        char input_str2[100];
	      printf("Input : ");
	      fflush(stdout);
        if(fgets(input_str2, sizeof(input_str2), stdin) == NULL)
            input_str2[0] = '\0';
        input_str2[strcspn(input_str2, "\n")] = '\0';
        strncat(concat_str, input_str2, sizeof(concat_str) - strlen(concat_str) - 1);

        // Close both reading ends 
        if(write(fd2[1], concat_str, strlen(concat_str) + 1) == -1)
        {
          fprintf(stderr, "Write Failed\n");
          exit(1);
        } 
        close(fd2[1]); 
        exit(0);
    } 
	return 0;
} 
