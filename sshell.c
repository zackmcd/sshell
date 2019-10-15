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
void trim_string(char *str);    //cut space at begin and end of the input
void cut_background(char *str); //cut '&' at the end of input
void ExecWithRedirector(cmd *cmd);
void ExcecWithPipe(cmd *process1);
void addJob(cmd *bgjob0, cmd *cmd);
void deleteJob(int pid, int status);
bool builtinCmds();
char buf[512];
bool background = false;
cmd *cmd0;
cmd *bgjob0;

void addJob(cmd *bgjob0, cmd *job)
{
  cmd *c = bgjob0;
  while (c->nextJob != NULL)
  {
    c = c->nextJob;
  }
  c->nextJob = job;
}

void deleteJob(int pid, int status)
{
  cmd *c = bgjob0;
  cmd *lastone = c;

  while (c != NULL && c->pid != pid)
  {
    lastone = c;
    c = c->nextJob;
  }
  if (c != NULL)
  {
    cmd_Completed(c, status);
    lastone->nextJob = c->nextJob;
  }
  if (lastone == bgjob0)
    bgjob0 = NULL;
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

  getline(&input, &size, stdin);

  //code for the tester
  if (!isatty(STDIN_FILENO))
  {
    printf("%s", input);
    fflush(stdout);
  }

  if (input[strlen(input) - 1] == '\n')
    input[strlen(input) - 1] = 0;
  cmd_setLine(cmd0, input);

  trim_string(input);
  cut_background(input);

  //check mislocated < > & errors of input line
  cmd_checkError(cmd0, input);
  if (cmd0->error)
    return;

  int beg = 0;
  int end = 0;
  bool check = true; // checks to see if there are any other args than just the command
  cmd *currentcmd = cmd0;

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
	if (i == 0) // tests if missing command before pipe
        {
          currentcmd->error = true;
	  fprintf(stderr, "Error: missing command\n");
	  continue;
        }

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

void trim_string(char *str)
{
  char *start, *end;
  int len = strlen(str);

  if (str[len - 1] == '\n')
  {
    len--;
    str[len] = 0;
  }
  start = str;         //point first char
  end = str + len - 1; //point last char
  while (*start && isspace(*start))
    start++;
  while (*end && isspace(*end))
    *end-- = 0;
  
  for (int i = 0; i < strlen(str); i++)
    str[i] = start[i];
}

void cut_background(char *str)
{
  char *end;
  int len = strlen(str);
  end = str + len - 1; //point last char
  if (*(end) == '&')
  {
    background = true;
    *end-- = 0;
  }
  while (*end && isspace(*end))
    *end-- = 0;
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
    int fd = open(cmd->outfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
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

bool builtinCmds()
{
  if (strcmp(cmd0->exec, "exit") == 0)
  {
    if (bgjob0 != NULL)
    {
      fprintf(stderr, "Error: active jobs still running\n");
      fprintf(stderr, "+ completed '%s' [%d]\n", "exit", 1);
    }
    else
    {
      fprintf(stderr, "Bye...\n");
      exit(0);
    }
    return true;
  }
  else if (strcmp(cmd0->exec, "cd") == 0)
  {
    int correct = 0;
    if (chdir(cmd0->args[1]) == -1)
    {
      fprintf(stderr, "Error: no such directory\n");
      correct = 1;
    }
    else
      fprintf(stderr, "+ completed '%s' [%d]\n", cmd0->line, correct);

    return true;
  }
  else if (strcmp(cmd0->exec, "pwd") == 0)
  {
    memset(buf, 0, sizeof(buf));
    getcwd(buf, sizeof(buf));
    printf("%s\n", buf);
    fprintf(stderr, "+ completed 'pwd' [0]\n");
    return true;
  }
  return false;
}

int main(int argc, char *argv[])
{
  while (1) //repeat forever
  {

    background = false;
    int status = 0;
    int zombieid = 0;
    display_prompt(); // Display prompt in terminal
    cmd0 = cmd_create();
    read_command(cmd0);

    while ((zombieid = waitpid(-1, &status, WNOHANG)) > 0)
    {
      deleteJob(zombieid, status);
    }
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

    //HANDLE BUILTIN COMMANDS
    if (builtinCmds())
      continue;

    pid_t pid = fork();
    if (pid != 0) //parent process
    {
      if (background)
      {
        if (bgjob0 == NULL)
          bgjob0 = cmd0;
        cmd0->pid = pid;
        addJob(bgjob0, cmd0);
        continue;
      }
      else
        waitpid(pid, &status, 0); // wait for child to exit
      //looking for zombie and print out
      while ((zombieid = waitpid(-1, &status, WNOHANG)) > 0)
      {
        deleteJob(zombieid, status);
      }

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
      cmd_Completed(cmd0, status);
      cmd_destroy(cmd0); //FREE mallocs
    }
    else
    {
      ExcecWithPipe(cmd0);
      exit(-1);
    }
  }
}
