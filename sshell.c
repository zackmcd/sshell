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

int main(int argc, char *argv[])
{
  while (1) //repeat forever
  {
    int status;   
    // int numcmds = 0;
    cmd *cmd0 = cmd_create();
    display_prompt(); // Display prompt in terminal
    read_command(cmd0);

    //TESTING
    printf("cmd1 is %s, infile is %s, outfile is %s\n", cmd0->exec, cmd0->infile, cmd0->outfile);
    for (int i = 0; i < 16; i++)
    {
      if (cmd0->args[i] != NULL)
        printf("arg%d is %s\n", i, cmd0->args[i]);
    }
    //TESTING
    
    if (cmd0->error)
    {
      //TODO : FREE ALL THOSE THINGS
      continue;
    }


    if (fork() != 0)
    {                                                               // fork off child process  Parent
      waitpid(-1, &status, 0);                                      // wait for child to exit
      fprintf(stderr, "+ completed %s [%d]\n", cmd0->line, status); // must print the entire command
    }
    else
    {
      ExcecWithPipe(cmd0);
      //perror("execvp");                // coming back here is an error
      exit(-1);
    }

    //}

    //FREE mallocs
    //   for (int i = 0; i <= numcmds; i++)
    //     free(cmds[i]);
    //   free(cmds);
    // }
  }
}

  void display_prompt()
  {
    printf("sshell$ ");
  }

  void read_command(cmd * cmd0)
  {
    char *input;
    input = (char *)malloc(CMD_MAX * sizeof(char));
    size_t size = CMD_MAX;
    int num = getline(&input, &size, stdin);
    input[num - 1] = '\0';

    int beg = 0;
    int end = 0;
    bool check = true; // checks to see if there are any other args than just the command
    cmd *currentcmd = cmd0;

    cmd_setLine(cmd0, input);

    for (int i = 0; i <= strlen(input); i++)
    {
      if (input[i] == '\0' && check)
      {
        cmd_setExec(currentcmd, input);
        cmd_addArg(currentcmd, input);
      }
      else if (isspace(input[i]) || input[i] == '\0' || input[i] == '|' || input[i] == '<' || input[i] == '>')
      {
        // printf("%d - %d = %d\n", end, beg, end - beg);
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

  void ExecWithRedirector(cmd * cmd)
  {
    if (cmd->infile)
    {
      int fd = open(cmd->infile, O_RDWR);
      printf("yes we opened infile, fd is %d\n", fd);
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

  void ExcecWithPipe(cmd * process1)
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
    {                      /* Parent*/
      close(fd[1]);        /* Don't need write access to pipe */
      dup2(fd[0],STDIN_FILENO);          /* And replace it with the pipe */
      close(fd[0]);        /* Close now unused file descriptor */
      if (process2->next != NULL)
        ExcecWithPipe(process2); //###### RECURSIVE HERE #####
      else
        ExecWithRedirector(process2);
    }
  }
