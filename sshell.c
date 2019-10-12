#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cmd.h"

#define CMD_MAX 512

void display_prompt();
void read_command(cmd *cmd0);
void ExecWithRedirector(cmd *cmd);
void ExcecWithPipe(cmd *process1);
char buf[512];
bool background = false;

int main(int argc, char *argv[])
{
  while (1) //repeat forever
  {
    int status;
    cmd *cmd0 = cmd_create();
    display_prompt(); // Display prompt in terminal
    read_command(cmd0);

    // //TESTING
    // printf("cmd1 is %s, infile is %s, outfile is %s\n", cmd0->exec, cmd0->infile, cmd0->outfile);
    // for (int i = 0; i < 16; i++)
    // {
    //   if (cmd0->args[i] != NULL)
    //     printf("arg%d is %s\n", i, cmd0->args[i]);
    // }
    // //TESTING
    cmd *check = cmd0;
    bool leave = false;
    while (check != NULL) // to test if there are any errors in any of the commands
    {
      if (check->error)
      {
        cmd_destroy(cmd0);
        leave = true;
        break;
      }
      else if (check->exec == NULL)
      {
        cmd_destroy(cmd0);
	leave = true;
	fprintf(stderr, "Error: command not found\n");
	break;
      }
      check = check->next;
    }

    if (leave)
      continue;

    //BUILTIN COMMANDS
    if (strcmp(cmd0->exec, "exit") == 0)
    {
      fprintf(stderr, "Bye...\n");
      exit(0);
    }
    else if (strcmp(cmd0->exec, "cd") == 0)
    {
      if(chdir(cmd0->args[1])==-1) 
        fprintf(stderr,"Error: no such directory\n");
      continue;
    }
    else if (strcmp(cmd0->exec, "pwd") == 0)
    {
      memset(buf, 0, sizeof(buf));
      getcwd(buf, sizeof(buf));
      printf("%s\n", buf);
      continue;
    }
    
    if (fork() != 0)
    {
      //if() wait(null) else
      waitpid(-1, &status, 0); // wait for child to exit

      if (status == 65280)
      {
        fprintf(stderr, "Error: command not found\n");
        status = 1;
      }
      else if (status == 256)
      {
        fprintf(stderr, "Error: no such directory\n");
        status = 1;
      }
      // error active jobs still running
      fprintf(stderr, "+ completed '%s'", cmd0->line); //[%d]", cmd0->line, status); //%d]\n" // must print the entire command
      cmd *c = cmd0->next;
      while(c != NULL)
      {
        fprintf(stderr, " [%d]", c->retval);
        c = c->next;
      }
      fprintf(stderr, " [%d]\n", status);
    }
    else
    {
      ExcecWithPipe(cmd0);
      exit(-1);
    }

    //}

    //FREE mallocs
    cmd_destroy(cmd0);
  }
}

void display_prompt()
{
  printf("sshell$ ");
}

void read_command(cmd *cmd0)
{
  char *input;
  input = (char *)malloc(CMD_MAX * sizeof(char));
  size_t size = CMD_MAX;
  int num = getline(&input, &size, stdin);
  input[num - 1] = '\0';
  
  //code for the tester
  if (!isatty(STDIN_FILENO))
  {
    printf("%s", input);
    fflush(stdout);
  }

  //check syntax errors of input line
  cmd_checkError(cmd0,input);
  if(cmd0->error) return;

  int beg = 0;
  int end = 0;
  bool check = true; // checks to see if there are any other args than just the command
  cmd *currentcmd = cmd0;

  cmd_setLine(cmd0, input);

  for (int i = 0; i <= strlen(input) && !currentcmd->error; i++) //added error so if its an error it will end process
  {
    if (input[i] == '\0' && check)
    {
      cmd_setExec(currentcmd, input);
      cmd_addArg(currentcmd, input);
    }
    else if (isspace(input[i]) || input[i] == '\0' || input[i] == '|' || input[i] == '<' || input[i] == '>')
    {
      if ((isspace(input[i - 1]) && isspace(input[i]))) // to deal with multiple spaces in command
      {
        beg++;
        end++;
        continue;
      }

      if (input[i] == '\0') // to handle the null terminator at the end of the command
        end++;

      check = false;
      int index = end - beg;
      char word[index];

      for (int j = 0; j < index; j++) // copies argument from command
      {
        word[j] = input[beg];
        beg++;
      }

      word[index] = '\0'; // adds null terminator in string
      if (index != 0)
      {
        if (currentcmd->exec == NULL) // if it is the command
        {
          cmd_setExec(currentcmd, word);
          cmd_addArg(currentcmd, word);
        }
        else if (currentcmd->input) // last word is in-redirection
        {
          cmd_setInFile(currentcmd, word);
          cmd_setIn(currentcmd, false);
        }
        else if (currentcmd->output) // last word is out-redirection
        {
          cmd_setOutFile(currentcmd, word);
          cmd_setOut(currentcmd, false);
        }
        else //if it is an argument
        {
          cmd_addArg(currentcmd, word);
        }
      }

      if (input[i] == '|')
      {
        currentcmd->next = cmd_create();
        currentcmd = currentcmd->next;
      }
      if (input[i] == '<')
      {
        cmd_setIn(currentcmd, true);
      }
      if (input[i] == '>')
      {
        cmd_setOut(currentcmd, true);
      }

      end++;
      beg++;
    }
    else // when its just a character in an argument
    {
      end++;
    }
  }
  // FREE ALL MALLOCS
  free(input);
}

void ExecWithRedirector(cmd *cmd)
{
  if (cmd->infile)
  {
    int fd = open(cmd->infile, O_RDWR);
    dup2(fd, STDIN_FILENO);
    close(fd);
  }
  if (cmd->outfile)
  {
    int fd = open(cmd->outfile, O_RDWR);
    dup2(fd, STDOUT_FILENO);
    close(fd);
  }
  execvp(cmd->exec, cmd->args);
}

void ExcecWithPipe(cmd *process1)
{
  if (process1->next == NULL)
  {
    ExecWithRedirector(process1);
    exit(-1);
  }

  cmd *process2 = process1->next;

  int fd[2];
  pipe(fd); /* Create pipe */
  if (fork() == 0)
  {                             /* Child*/
    close(fd[0]);               /* Don't need read access to pipe */
    dup2(fd[1], STDOUT_FILENO); /* Replace stdout with the pipe */
    close(fd[1]);               /* Close now unused file descriptor */
    ExecWithRedirector(process1);
    exit(-1);
  }
  else
  {                            /* Parent*/
    close(fd[1]);              /* Don't need write access to pipe */
    dup2(fd[0], STDIN_FILENO); /* And replace it with the pipe */
    close(fd[0]);              /* Close now unused file descriptor */
    
    int val;
    waitpid(-1, &val, 0); // wait for child to exit
    cmd_setRetval(process1, val);
    
    if (process2->next != NULL)
      ExcecWithPipe(process2); //###### RECURSIVE HERE #####
    else
      ExecWithRedirector(process2);
  }
}
