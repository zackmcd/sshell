#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void display_prompt();
void read_command(char ***command);

int main(int argc, char *argv[])
{

  //while (1) //repeat forever 
  //{
    //char *command[3] = {"/bin/date", "-u", NULL};
    int status;
    //char **command;
    display_prompt();                 // Display prompt in terminal     
    //read_command(&command);           // Read input from terminal 
    char *input;
    size_t size = 512;
    input = (char *)malloc(size * sizeof(char));
    getline(&input, &size, stdin);
    printf("%s wow", input);
    char *command[2] = {"date", NULL};

    if (fork() != 0)
    {                                 // fork off child process  Parent       
      waitpid(-1, &status, 0);        // wait for child to exit  
      fprintf(stderr, "+ completed %s [%d]\n", command[0], status);
    }
    else
    {
      execv(command[0], command);     // execute command
      perror("execv");                // coming back here is an error 
      exit(1);
    }
  //}
}

void display_prompt()
{
  printf("sshell$ ");
}

void read_command(char ***command)
{
  char *input;
  size_t size = 512;
  //size_t numChars;
  input = (char *)malloc(size * sizeof(char));
  //numChars = getline(&input, &size, stdin);
  getline(&input, &size, stdin);


  *command = malloc(sizeof(char*) * 16);
  for (int i = 0; i < 16; i++)
  {
    *command[i] = malloc(sizeof(char) * 16);
  }
  //for TESTING

  strcpy(input, *command[0]);
  printf("3%s", *command[0]);
  *command[1] = NULL;

  /*
  for (int i = 0; i < numChars; i++)
  {
    if (!isspace(numChars[i]))
    {

}
