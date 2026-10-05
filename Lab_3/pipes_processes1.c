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
  
    printf("Enter a string to concatenate:");
    scanf("%s", input_str); 
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
        write(fd1[1], input_str, strlen(input_str)+1); 
        close(fd1[1]);
  
        // Wait for child to print the concatenated string 
        wait(NULL); 

	char concat_str[300];
	read(fd2[0], concat_str, sizeof(concat_str));
	close(fd2[0]);

	strcat(concat_str, fixed_str));
	printf("Concentrated string %s\n", concat_str);
    } 
  
    // child process 
    else
    { 
        close(fd1[1]);  // Close writing end of first pipes 
        close(fd2[0]); 
      
        // Read a string using first pipe 
        char concat_str[300]; 
        read(fd1[0], concat_str, 100);
	close(fd1[0]); 
  
        // Concatenate a fixed string with it 
        int k = strlen(concat_str); 
        int i; 
        for (i=0; i<strlen(fixed_str); i++) 
            concat_str[k++] = fixed_str[i]; 
  
        concat_str[k] = '\0';   // string ends with '\0' 
  
        printf("Concatenated string %s\n", concat_str);

	char input_str2[100];
	printf("Enter a string tp concatenate:");
	fflush(stdout);
	scanf("%99s", input_str2);
	strcat(concat_str, input_str2);

        // Close both reading ends 
        write(fdr2[1], concat_str, strlen(concat_str) + 1); 
        close(fd2[1]); 

  
        exit(0); 
    } 
	return0;
} 
